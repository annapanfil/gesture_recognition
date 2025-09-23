#include "cnn.h"

CNNImpl::CNNImpl() {
        conv1 = register_module("conv1", torch::nn::Conv2d(1, 16, 3));  // 255x255x1 -> 253x253x16
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(16)); 
        relu1 = register_module("relu1", torch::nn::ReLU());  
        pool1 = register_module("pool1", torch::nn::MaxPool2d(2)); // 126x126x16

        conv2 = register_module("conv2", torch::nn::Conv2d(16, 32, 3)); // 124x124x32
        bn2 = register_module("bn2", torch::nn::BatchNorm2d(32));
        relu2 = register_module("relu2", torch::nn::ReLU());
        pool2 = register_module("pool2", torch::nn::MaxPool2d(2)); // 62x62x32

        flatten = register_module("flatten", torch::nn::Flatten()); // 32*62*62
        dropout = register_module("dropout", torch::nn::Dropout(0.5));
        fc1 = register_module("fc1", torch::nn::Linear(32 * 62 * 62, 14)); // 14 classes
        softmax = register_module("softmax", torch::nn::LogSoftmax(1));
    }

torch::Tensor CNNImpl::forward(torch::Tensor x){
        x = pool1->forward(relu1->forward(bn1->forward(conv1->forward(x))));
        x = pool2->forward(relu2->forward(bn2->forward(conv2->forward(x))));
        x = flatten->forward(x);
        x = dropout->forward(x);
        x = fc1->forward(x);
        x = softmax->forward(x);
        return x;
    }