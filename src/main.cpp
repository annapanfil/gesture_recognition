#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
    cv::Mat image = cv::imread("/home/anna/Pictures/litter.png");
    if (image.empty()){
        std::cerr << "Couldn't read the photo\n";
        return 1;
    }

    cv::Mat preprocessed_image;
    cv::resize(image, preprocessed_image, cv::Size(255, 255), 0, 0);
    cv::cvtColor(preprocessed_image, preprocessed_image, cv::COLOR_BGR2GRAY);
    preprocessed_image.convertTo(preprocessed_image, CV_32F, 1/255.0);

    cv::imshow("Image after preprocessing", preprocessed_image);
    cv::waitKey(0);

    return 0;

}