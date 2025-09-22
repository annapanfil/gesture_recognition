#pragma once
#include <torch/torch.h>

struct CNN : torch::nn::Module{
    CNN();
    torch::Tensor forward(torch::Tensor x);

    torch::nn::Conv2d conv1{nullptr}, conv2{nullptr};
    torch::nn::BatchNorm2d bn1{nullptr}, bn2{nullptr};
    torch::nn::ReLU relu1{nullptr}, relu2{nullptr};
    torch::nn::MaxPool2d pool1{nullptr}, pool2{nullptr};
    torch::nn::Flatten flatten{nullptr};
    torch::nn::Dropout dropout{nullptr};
    torch::nn::Linear fc1{nullptr};
    torch::nn::LogSoftmax softmax{nullptr};
};