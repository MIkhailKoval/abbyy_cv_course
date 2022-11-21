#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>

namespace {
int window_size = 3;
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
        cv::Vec3b& GetCell(int x, int y) {
            x = std::abs(x);
            y = std::abs(y);
            if (x >= GetRows()) {
                x = 2 * GetRows() - 1 - x;
            }
            if (y >= GetCols()) {
                y = 2 * GetCols() - 1 - y;
            }
            return image->at<cv::Vec3b>(x,y);
        }
};
} // namespace

void calculate_and_update_median(ImageImpl& out_image, std::vector<std::vector<int>>& hist, int center_x, int center_y) {
    int median_number = (window_size * window_size - 1) / 2 + 1;
    int inner_sum[3] = {0, 0, 0};
    int last_median[3] = {0, 0, 0};
    int counted_median = 0;
    for (size_t num = 0; num < hist[0].size(); num++) {
        for (size_t channel = 0; channel < 3; channel++) {
            if (inner_sum[channel] >= median_number || hist[channel][num] == 0) continue;
            if (hist[channel][num] + inner_sum[channel] >= median_number) {
                counted_median++;
                last_median[channel] = num;
            }
            inner_sum[channel] += (int)hist[channel][num];
        }
        if (counted_median == 3) break;
    }
    if (counted_median != 3) {
        std::cout << "median counted badly " << counted_median << '\n';
        std::cout << inner_sum[0] << ' ' << median_number << '\n';
    }
    cv::Vec3b& central_cell = out_image.GetCell(center_x, center_y);
    for (int channel = 0; channel < 3; channel++) {
        central_cell[channel] = last_median[channel];
    }
}

void median_filter_best(ImageImpl& in_image, ImageImpl& out_image) {
    std::vector<std::vector<std::vector<int> >> all_hists(in_image.GetCols(), std::vector<std::vector<int>>(3, std::vector<int>(256, 0)));
    // calculate hists
    for (int i = 0; i < window_size; i++) {
        for (int j = 0; j < in_image.GetCols(); j++) {
            cv::Vec3b& cell = in_image.GetCell(i, j);
            for (int channel = 0; channel < 3; channel++) {
                all_hists[j][channel][cell[channel]]++;
            }
        }
    }

    for (int i = 0; i + window_size < in_image.GetRows(); i++) {
        std::vector<std::vector<int>> current_hist(3, std::vector<int>(256, 0));
        if (i != 0) {
            // update column
            for (int j = 0; j < window_size; j++) {
                cv::Vec3b& old_cell = in_image.GetCell(i - 1, j);
                cv::Vec3b& new_cell = in_image.GetCell(i + window_size - 1, j);
                for (int channel = 0; channel < 3; channel++) {
                    if (all_hists[j][channel][old_cell[channel]] <= 0) {
                        std::cout << "here: algorithm was crushed\n";
                    }
                    all_hists[j][channel][old_cell[channel]]--;
                    all_hists[j][channel][new_cell[channel]]++;
                }
            }
        }

        // calculate current_hist
        for (int num = 0; num < 256; num++) {
            for (int j = 0; j < window_size; j++) {
                for (int channel = 0; channel < 3; channel++) {
                    current_hist[channel][num] += all_hists[j][channel][num];
                }
            } 
        }

        int center_x = i + (window_size - 1) / 2;
        int center_y = (window_size - 1) / 2;
        for (int j = 0; j + window_size < in_image.GetCols(); j++) {
            if (i != 0) {
                // update column
                cv::Vec3b& old_cell = in_image.GetCell(i - 1, j + window_size);
                cv::Vec3b& new_cell = in_image.GetCell(i + window_size - 1, j + window_size);
                for (int channel = 0; channel < 3; channel++) {
                    if (all_hists[j + window_size][channel][old_cell[channel]] <= 0) {
                        std::cout << "algorithm was crushed\n";
                    }
                    all_hists[j + window_size][channel][old_cell[channel]]--;
                    all_hists[j + window_size][channel][new_cell[channel]]++;
                }
            }
            if (j != 0) {
                // update current hist
                for (int channel = 0; channel < 3; channel++) {
                    auto& old_hist = all_hists[j - 1][channel];
                    auto& new_hist = all_hists[j + window_size - 1][channel];
                    for (int num = 0; num < 256; num++) {
                        current_hist[channel][num] += (new_hist[num] - old_hist[num]);
                        if (current_hist[channel][num] < 0) {
                            std::cout << "algorithm crushed\n";
                            std::cout << i << ' ' << j << ' '  << channel << ' ' << num << '\n';

                            return;
                        }
                    }
                }
            }
            calculate_and_update_median(out_image, current_hist, center_x, center_y);
            center_y++;
        }
    }
}

void median_filter_better(ImageImpl& in_image, ImageImpl& out_image) {
    for (int curr_i = 0; curr_i + window_size < in_image.GetRows(); curr_i++) {
        // init histogram
        std::vector<std::vector<int> > hist(3, std::vector<int>(256, 0));

        for (int delta_i = 0; delta_i < window_size; delta_i++) {
            for (int j = 0; j < window_size; j++) {
                cv::Vec3b& cell = in_image.GetCell(curr_i + delta_i, j);
                for (int channel = 0; channel < 3; channel++) {
                    hist[channel][(int)cell[channel]]++;
                }
            }
        }
        int center_x = curr_i + (window_size - 1) / 2;
        int center_y = (window_size - 1) / 2;
        calculate_and_update_median(out_image, hist, center_x, center_y);

        // calculate next columns
        for (int j = window_size; j < in_image.GetCols(); j++) {
            // update histogram
            for (int delta_i = 0; delta_i < window_size; delta_i++) {
                auto old_cell = in_image.GetCell(curr_i + delta_i, j - window_size);
                auto new_cell = in_image.GetCell(curr_i + delta_i, j);
                for (int channel = 0; channel < 3; channel++) {
                    hist[channel][(int)old_cell[channel]]--;
                    hist[channel][(int)new_cell[channel]]++;
                }
            }
            center_y++;
            calculate_and_update_median(out_image, hist, center_x, center_y);
        }
    }
}

void median_filter(ImageImpl& in_image, ImageImpl& out_image) {
    for (int i = 0; i + window_size < in_image.GetRows(); i++) {
        for (int j = 0; j + window_size < in_image.GetCols(); j++) {
            std::vector<int> b, g, r;
            for (int x = i; x < i + window_size; x++) {
                for (int y = j; y < j + window_size; y++) {
                    auto cell = in_image.GetCell(x,y);
                    b.push_back(cell[0]);
                    g.push_back(cell[1]);
                    r.push_back(cell[2]);
                }
            }
            std::sort(b.begin(), b.end());
            std::sort(g.begin(), g.end());
            std::sort(r.begin(), r.end());
            
            int center_x = i + (window_size - 1) / 2, center_y = j + (window_size - 1) / 2;
            cv::Vec3b& central_cell = out_image.GetCell(center_x,center_y);
            central_cell[0] = b[(b.size() - 1) / 2];
            central_cell[1] = g[(b.size() - 1) / 2];
            central_cell[2] = r[(b.size() - 1) / 2];
        }
    }
}

void check_equal(ImageImpl& first, ImageImpl& second) {
    for (int i = 0; i < first.GetRows(); i++) {
        for (int j = 0; j < first.GetCols(); j++) {
            cv::Vec3b& cell_1 = first.GetCell(i,j);
            cv::Vec3b& cell_2 = second.GetCell(i,j);
            for (int channel = 0; channel < 3; channel++) {
                if (cell_1[channel] != cell_2[channel]) {
                    std::cout << i << ' ' << j << ' ' << channel << " :  " << (int)cell_1[channel] << ' ' << (int)cell_2[channel] << '\n';
                    cell_2[2] = 255;
                    cell_2[1] = cell_2[0] = 0;
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    if ( argc != 4 )
    {
        printf("usage: ./build/Sort <Image_Path> best \n");
        return -1;
    }
    cv::Mat input_image, output_image;
    input_image = cv::imread(argv[1], 1);
    output_image = cv::imread(argv[1], 1);
    if (!input_image.data || !output_image.data) {
        printf("No image data \n");
        return -1;
    }
    ImageImpl impl_in_image{input_image},
        impl_out_image{output_image};
    
    std::string algo_type = argv[2];
    window_size = std::stoi(argv[3]);
    unsigned int start_time =  clock();
    if (algo_type == "naive") {
        median_filter(impl_in_image, impl_out_image);
    } else if (algo_type == "better") {
        median_filter_better(impl_in_image, impl_out_image);
    } else {
        median_filter_best(impl_in_image, impl_out_image);
    }
    unsigned int end_time =  clock();
    std::cout.precision(4);
    std::cout << 1.0 * (end_time - start_time) / CLOCKS_PER_SEC;
    cv::imwrite("pictures/window_size=" + std::to_string(window_size) +".bmp", output_image);
    return 0;
} 