#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>

using namespace std;
namespace {
    std::string name = "0";
class ImageImpl {
    private:
        cv::Mat* image;
    public:
        ImageImpl(cv::Mat& image) : image(&image)  {
        }
        int GetCols() const {
            return image->cols;
        }
        int GetRows() const {
            return image->rows;
        }
        uchar& GetCell(int x, int y) {
            return image->at<uchar>(x,y);
        }
        cv::Mat& GetImage() const {
            return *image;
        }
};
} // namespace

cv::Mat rotate(cv::Mat src, double angle) {
    cv::Mat dst;      
    cv::Point2f pt(src.cols/2., src.rows/2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(src, dst, r, cv::Size(src.cols, src.rows));
    return dst;
}

std::vector<std::vector<long long>> calculateFHTrec(
        ImageImpl& image, int xmin, int xmax) {
    std::vector<std::vector<long long>> res(image.GetCols(), std::vector<long long>(xmax - xmin, 0));
    if (xmax - xmin == 1) {
        for (int x = 0; x < image.GetCols(); x++) {
            res[x][0] = image.GetCell(x, xmin);
        }
        return res;
    }
    int mid = (xmin + xmax) / 2;
    auto ans1 = calculateFHTrec(image, xmin, mid);
    auto ans2 = calculateFHTrec(image, mid, xmax);
    for (int x = 0; x < image.GetCols(); x++) {
        for (int shift = 0; shift < xmax-xmin; shift++) {
            res[x][shift] = ans1[x][shift / 2] + ans2[(x + (shift + 1) / 2) % image.GetCols()][shift / 2];
        }
    }
    return res;
}

std::vector<std::vector<long long>> calculateFHT(ImageImpl& image, bool is_reversed) {
    int size = 1;
    int real_size = image.GetRows();
    while (size * 2 < real_size) {
        size *= 2;
    }
    cv::Mat image_1, out;
    ImageImpl impl{image_1}, result{out};
    impl.GetImage().create(size, image.GetCols(), CV_8U);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < image.GetCols(); j++) {
            if (i < image.GetRows()) {
                impl.GetCell(i, j) = image.GetCell(i, is_reversed ? image.GetCols() - j - 1: j);
            } else {
                impl.GetCell(i, j) = 0;
            }
        }
    }
    auto res = calculateFHTrec(impl, 0, size);

    result.GetImage().create(res.size(), res[0].size(), CV_8U);
    long long mx{0};
    for (size_t i = 0; i < res.size(); i++) {
        for (size_t j = 0; j < res[0].size(); j++) {
            mx = std::max(mx, res[i][j]);
        }
    }

    for (int i = 0; i < result.GetRows(); i++) {
        for (int j = 0; j < result.GetCols(); j++) {
            auto& value = result.GetCell(i, j);
            value = 1.0 * res[i][j] * 255 /  mx;
            res[i][j] = value;
        }
    }
    //cv::imwrite("result/" + name + "_" + "haff" + (is_reversed? "_reversed": "") +".jpg", out);
    return res;
}

double calculateAngle(std::vector<std::vector<std::vector<long long>>>& res) {
    int size = res[0].size();
    int axis_size = 2;
    std::cout << "here" << std::endl;
     // get rid of denominator, because it is equal for all elements
    std::vector<std::vector<long long>> vars(size, std::vector<long long>(2, 0));
    for (int is_reversed = 0; is_reversed < axis_size; is_reversed++) {
        for (int i = 0; i < size; i++) {
            auto n = res[is_reversed][i].size();
            long long sum{0};
            for (size_t j = 0; j < res[is_reversed][i].size(); j++) {
                sum += res[is_reversed][i][j];
            }
            for (size_t j = 0; j < res[is_reversed][i].size(); j++) {
                vars[i][is_reversed] += (n * res[is_reversed][i][j] - sum) * (n * res[is_reversed][i][j] - sum);
            }
        }
    }
    long long max_var{0}, argmx = 0, sign = 1;
    for (int is_reversed = 0; is_reversed < axis_size; is_reversed++) {
        for (size_t i = 0; i < vars.size(); i++) {
            max_var = std::max(max_var, vars[i][is_reversed]);
        } 
    }
    for (size_t i = 0; i < vars.size(); i++) {
        for (int v = 0; v < axis_size; v++) {
            if (max_var == vars[i][v]) {
                argmx = i;
                sign = - 2 * v + 1;
                break;
            }
        }
    } 
    double angle = atan(1.0 * argmx / size) * 180 / M_PI * sign;

    std::cout.precision(4);
    std::cout << "argmx: " << argmx << std::endl;
    std::cout << "angle " << ' ' << angle << std::endl;
    return angle;
}

void calculateSobel(ImageImpl& image, ImageImpl& out) {
    cv::Mat src;
    cv::GaussianBlur(image.GetImage(), src, cv::Size(25, 25), 0, 0, cv::BORDER_DEFAULT);
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;
    Sobel(src, grad_x, CV_16S, 1, 0, /*ksize*/ 1, /*scale*/ 1, /*delta*/ 0, cv::BORDER_DEFAULT);
    Sobel(src, grad_y, CV_16S, 0, 1, /*ksize*/ 1, /*scale*/ 1, /*delta*/ 0, cv::BORDER_DEFAULT);
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, out.GetImage());
}

void FixPicture() {
    auto input_image = cv::imread("images/" + name + ".jpg", cv::IMREAD_GRAYSCALE);
    if (!input_image.data) {
        printf("No image data \n");
        return;
    }
    ImageImpl impl_in_image_1{input_image};
    unsigned int start_time = clock();

    cv::Mat sobel;
    ImageImpl impl_sobel{sobel};
    calculateSobel(impl_in_image_1, impl_sobel);
    auto res = calculateFHT(impl_sobel, false);
    auto flipped_res = calculateFHT(impl_sobel, true);
    std::vector<std::vector<std::vector<long long>>> vec{res, flipped_res};
    auto angle = calculateAngle(vec);

    auto dst = rotate(input_image, angle);

    unsigned int end_time = clock();
    std::cout.precision(4);
    cv::imwrite("result/" + name + "_" + "result.jpg", dst);
    std::cout << 1.0 * (end_time - start_time) / CLOCKS_PER_SEC << std::endl;
    std::cout << angle << std::endl;
}

int main(int argc, char** argv) {
    if ( argc != 1 ) {
        printf("usage: ./build/Sort \n");
        return -1;
    }
    for (int i = 1; i <= 10; i++) {
        name = std::to_string(i);
        FixPicture();
    }
    return 0;
}
