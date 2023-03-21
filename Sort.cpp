#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

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

void AddPaper(cv::Mat& image, int num, int k = 60) {
    cv::Mat image_1, dst;
    image_1 = cv::imread("table_" + std::to_string(num) + ".jpg", cv::IMREAD_COLOR);
    if (!image_1.data) {
        printf("No image data \n");
        return;
    }
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            for (int color = 0; color < 3; color++) {
                if (i >= image_1.rows || j >= image_1.cols) continue;
                image.at<cv::Vec3b>(i, j)[color] = ((100 - k) * image.at<cv::Vec3b>(i, j)[color] + k * image_1.at<cv::Vec3b>(i, j)[color]) / 100;
            }
        }
    }
}

void Reflect(cv::Mat& image) {

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; 2 * j + 1 < image.cols; j++) {
            for (int color = 0; color < 3; color++) {
                uchar x = image.at<cv::Vec3b>(i, j)[color];
                uchar y = image.at<cv::Vec3b>(i, image.cols - 1 - j)[color];
                image.at<cv::Vec3b>(i, image.cols - 1 - j)[color] = x;
                image.at<cv::Vec3b>(i, j)[color] = y;
            }
        }
    }
}

void ReflectColors(cv::Mat& image) {

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            for (int color = 0; color < 3; color++) {
                image.at<cv::Vec3b>(i, j)[color] = 255 - image.at<cv::Vec3b>(i, j)[color];
            }
        }
    }
}

void AddVerticalLine(cv::Mat& image, bool is_black) {

    auto j = std::rand() % image.cols;
    for (int i = 0; i < image.rows; i++) {
        for (int f = j; f < image.cols && f - j < 15; f++) {
            for (int color = 0; color < 3; color++) {
                image.at<cv::Vec3b>(i, f)[color] = (is_black)? 250: 10;
            }
        }
    }
}

void AddGauss(cv::Mat& image, double del) {

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            double x = 0;
            for (int i = 0; i < 12; i++) {
                x += (std::rand() / 2147483647.0);
            }
            for (int color = 0; color < 3; color++) {
                image.at<cv::Vec3b>(i, j)[color] += (x / del);
            }
        }
    }
}

void ChangeContrast(cv::Mat& image, double alpha, int beta, bool should_make_strips) {
    for (int i = 0; i < image.rows; i++) {
        if (should_make_strips && std::rand() % 10 > 5) continue;
        for (int j = 0; j < image.cols; j++) {
            for (int color = 0; color < 3; color++) {
                image.at<cv::Vec3b>(i, j)[color] = cv::saturate_cast<uchar>(alpha * image.at<cv::Vec3b>(i, j)[color] + beta);
            }
        }
    }
}

void ProducePictures() {
    cv::Mat image, output;
    image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
    if (!image.data) {
        printf("No image data \n");
        return;
    }
    int num = 1;
    
    // #1 этап - поворот + бумага
    {
        image = rotate(image, 20);
        AddPaper(image, num);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }
    // #2 этап - бумага
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        AddPaper(image, num);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }
    // #3 этап - бумага + гауссовский шум
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        AddGauss(image, 7);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #4 этап - меняем контрастность и яркость
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        ChangeContrast(image, 2.0, 60, false);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #5 этап - добавление горизонтальных полос
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        ChangeContrast(image, 2.0, 60, true);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }
    // #6  этап - контрастность + гаус + бумага
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        ChangeContrast(image, 2.2, 0, false);
        AddPaper(image, num, 30);
        AddGauss(image, 1.5);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #7  этап - отражение
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        Reflect(image);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #8  этап - негатив + шум
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        ReflectColors(image);
        AddGauss(image, -6);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #9  этап - медианный фильтр с экспериментами
    {
        cv::Mat h_image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        AddGauss(h_image, 7);
        ChangeContrast(image, 1.0, 100, true);
        cv::GaussianBlur(h_image, image, cv::Size(3, 51), 0);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }

    // #10  этап - полосы
    {
        image = cv::imread("images/" + name + ".png", cv::IMREAD_COLOR);
        AddGauss(image, 7);
        for (int i = 0; i < 5; i++) {
            AddVerticalLine(image, /*is_black*/ std::rand() % 2);
        }
        cv::Mat h_image;
        cv::GaussianBlur(image, h_image, cv::Size(25, 25), 0);
        cv::imwrite("result/" + name + "#" + std::to_string(num) + ".png", image);
        num++;
    }
}

int main(int argc, char** argv) {
    std::srand(std::time(nullptr));
    if ( argc != 1 ) {
        printf("usage: ./build/Sort \n");
        return -1;
    }
    for (int i = 1; i <= 10; i++) {
        name = std::to_string(i);
        ProducePictures();
    }
    return 0;
}