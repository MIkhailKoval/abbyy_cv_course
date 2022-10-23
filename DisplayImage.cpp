#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>

namespace channels {
    static const int kBlue = 0;
    static const int kGreen = 1;
    static const int kRed  = 2;

    int get_cell_type(int i, int j) {
        if (i % 2 == 0 && j % 2 == 0) return kRed;
        if (i % 2 != 0 && j % 2 != 0) return kBlue;
        if (i % 2 == 0 && j % 2 != 0) return kGreen;
        if (i % 2 != 0 && j % 2 == 0) return kGreen;
        throw std::logic_error("unimplemented");
    }
} // namespace channels

int get_color(int i, int j, cv::Mat& image, int color) {
    return static_cast<int>(image.at<cv::Vec3b>(i,j)[color]);
}

int hue_transit(int l1, int l2, int l3, int v1, int v3) {
    if ((l1 < l2 && l2 < l3) || (l1 > l2 && l2 > l3)) {
        return v1 + (v3 - v1) * (l2 - l1) / (l3 - l1);
    } else {
        return (v1 + v3) / 2.0 + (l2 - (l1 + l3) / 2.0) / 2.0;
    }
}

void restore_colors(cv::Mat& image) {
    // restore green color

    for (int i = 2; i < image.rows - 2; i++) {
        for (int j = 2; j < image.cols - 2; j++) {
            int current_color = channels::get_cell_type(i, j);
            if (current_color != channels::kGreen) {
                int rb13 = get_color(i, j, image, current_color);
                int rb3 = get_color(i - 2, j, image, current_color);
                int rb11 = get_color(i, j - 2, image, current_color);
                int rb15 = get_color(i, j + 2, image, current_color);
                int rb23 = get_color(i + 2, j, image, current_color);

                int g8 = get_color(i - 1, j, image, channels::kGreen);
                int g12 = get_color(i, j - 1, image, channels::kGreen);
                int g14 = get_color(i, j + 1, image, channels::kGreen);
                int g18 = get_color(i + 1, j, image, channels::kGreen);

                int delta_n = std::abs(rb13 - rb3) * 2 + std::abs(g18 - g8);
                int delta_e = std::abs(rb13 - rb15) * 2 + std::abs(g12 - g14);
                int delta_w = std::abs(rb13 - rb11) * 2 + std::abs(g14 - g12);
                int delta_s = std::abs(rb13 - rb23) * 2 + std::abs(g8 - g18);
                
                auto delta = std::min(delta_n, std::min(delta_e, std::min(delta_w, delta_s)));
                int g;
                if (delta_n == delta) {
                    g = (g8 * 3 + g18 + rb13 - rb3) / 4;
                } else if (delta_e == delta) {
                    g = (g14 * 3 + g12 + rb13 - rb15) / 4;
                } else if (delta_w == delta) {
                    g = (g12 * 3 + g14 + rb13 - rb11) / 4;
                } else {
                    g = (g18 * 3 + g8 + rb13 - rb23) / 4;
                }
                image.at<cv::Vec3b>(i,j)[channels::kGreen] = std::max(0, std::min(255, g));
            }
        }
    }

    // restore red and blue
    for (int i = 1; i < image.rows - 1; i++) {
        for (int j = 1; j < image.cols - 1; j++) {
            if (channels::get_cell_type(i, j) == channels::kGreen) {
                int g7 = get_color(i, j - 1, image, channels::kGreen);
                int g8 = get_color(i, j, image, channels::kGreen);
                int g9 = get_color(i, j + 1, image, channels::kGreen);

                int right_color = channels::get_cell_type(i, j + 1);
                int high_color = channels::get_cell_type(i - 1, j);
                if (right_color == channels::kGreen || high_color == channels::kGreen || high_color == right_color) {
                    throw std::logic_error("bad colors");
                }

                int g3 = get_color(i - 1, j, image, channels::kGreen);
                int g13 = get_color(i + 1, j, image, channels::kGreen);

                int br7 = get_color(i, j - 1, image, right_color);
                int br9 = get_color(i, j + 1, image, right_color);

                int rb3 = get_color(i - 1, j, image, high_color);
                int rb13 = get_color(i + 1, j, image, high_color);

                auto br = hue_transit(g7, g8, g9, br7, br9);
                auto rb = hue_transit(g3, g8, g13, rb3, rb13);

                image.at<cv::Vec3b>(i,j)[right_color] = std::max(0, std::min(255, br));
                image.at<cv::Vec3b>(i,j)[high_color] = std::max(0, std::min(255, rb));
            }
        }
    }
    // restore red or blue
    for (int i = 2; i < image.rows - 2; i++) {
        for (int j = 2; j < image.cols - 2; j++) {
            auto current_color = channels::get_cell_type(i, j);
            auto another_color = channels::get_cell_type(i - 1, j - 1);
            int g13 = get_color(i, j, image, channels::kGreen);

            if (current_color != channels::kGreen) {
                if (current_color == channels::kGreen || another_color == channels::kGreen || another_color == current_color) {
                    throw std::logic_error("bad colors");
                }
                int r1 = get_color(i - 2, j - 2, image, current_color);
                int b7 = get_color(i - 1, j - 1, image, another_color);
                int r13 = get_color(i, j, image, current_color);
                int b19 = get_color(i + 1, j + 1, image, another_color);
                int r25 = get_color(i + 2, j + 2, image, current_color);
                
                int r5 = get_color(i - 2, j + 2, image, current_color);
                int b9 = get_color(i - 1, j + 1, image, another_color);
                int b17 = get_color(i + 1, j - 1, image, another_color);
                int r21 = get_color(i + 2, j - 2, image, current_color);

                
                int g7 = get_color(i - 1, j - 1, image, channels::kGreen);
                int g9 = get_color(i - 1, j + 1, image, channels::kGreen);
                int g17 = get_color(i + 1, j - 1, image, channels::kGreen);
                int g19 = get_color(i + 1, j + 1, image, channels::kGreen);


                int ne = std::abs(b9 - b17) + std::abs(r5 - r13) + std::abs(r13 - r21) + std::abs(g9 - g13) + std::abs(g13 - g17);
                int nw = std::abs(b7 - b19) + std::abs(r1 - r13) + std::abs(r13 - r25) + std::abs(g7 - g13) + std::abs(g13 - g19);

                if (ne < nw) {
                    auto r = hue_transit(g9, g13, g17, b9, b17);
                    image.at<cv::Vec3b>(i,j)[another_color] = std::max(0, std::min(255, r));
                } else {
                    auto r = hue_transit(g7, g13, g19, b7, b19);
                    image.at<cv::Vec3b>(i,j)[another_color] = std::max(0, std::min(255, r));
                }  
            }
        }
    }
}

int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: ./DisplayImage <Image_Path>\n");
        return -1;
    }
    cv::Mat expected, new_image, image;

    image = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    cv::cvtColor(image, new_image, cv::COLOR_GRAY2BGR);
    restore_colors(new_image);
    cv::imwrite("result.bmp", new_image);
    return 0;
} 