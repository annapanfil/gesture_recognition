#include <iostream>
#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include "cnn.hpp"

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

    torch::Tensor tensor = torch::from_blob(
        image.data, {1, image.rows, image.cols}, torch::kFloat32); // C, H, W

    tensor = tensor.unsqueeze(0); // B, C, H, W

    std::cout << "Input tensor size: " << tensor.sizes() << std::endl;

    CNN model = CNN();
    torch::NoGradGuard no_grad;
    model.eval();

    torch::Tensor output = model.forward(tensor);
    int64_t pred = output.argmax(1).item<int64_t>();
    
    std::cout << "Output: " << output << std::endl;
    std::cout << "Predicted class: " << pred << std::endl;

    return 0;
}