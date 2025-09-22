#include<iostream>
#include<opencv2/opencv.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;
    cv::Mat image = cv::imread("/home/anna/Pictures/litter.png");
    if (image.empty()){
        std::cerr << "Couldn't read the photo\n";
        return 1;
    }

    cv::resize(image, image, cv::Size(255, 255), 0, 0);
    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
    image.convertTo(image, CV_32F, 1/255.0);

    cv::imshow("Image", image);
    cv::waitKey(0);

    return 0;
}