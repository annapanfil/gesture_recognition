#include <iostream>
#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include "cnn.h"
#include "gesture_dataset.h"

void train_model(CNN& model){
    model->train();
    const size_t batch_size = 64;
    std::string data_root = "../data/HG14/HG14-Hand-Gesture/";
    int img_size = 255;

    auto dataset = GestureDataset(data_root, img_size)
        .map(torch::data::transforms::Stack<>());

    size_t dataset_size = dataset.size().value();

    auto dataloader = torch::data::make_data_loader(
        std::move(dataset),
        torch::data::samplers::RandomSampler(dataset.size().value()),
        batch_size
    ); /*std::unique_ptr*/

    auto criterion = torch::nn::CrossEntropyLoss();
    torch::optim::Adam optimizer(model->parameters(), torch::optim::AdamOptions(1e-3));

    for (size_t epoch = 0; epoch < 3; ++epoch) {
        size_t batch_idx = 0;
        float epoch_loss = 0.0;
        
        for (auto& batch : *dataloader) {
            optimizer.zero_grad();
            torch::Tensor output = model->forward(batch.data); // B x num_classes
            torch::Tensor loss = criterion(output, batch.target);
            loss.backward();
            optimizer.step();

            epoch_loss += loss.item<float>();

            if (batch_idx++ % 10 == 0) {
                std::cout << "Epoch [" << epoch << "] Batch [" << batch_idx << "/" << dataset_size / batch_size << "] Current loss: " << loss.item<float>() << "\n";
            }
        }
        std::cout << "Epoch [" << epoch << "] Loss: " << epoch_loss / batch_size << "\n";
    }

    torch::save(model, "../models/cnn_trained.pt");
    std::cout << "Model trained and saved to ../models/cnn_trained.pt\n";

}


int64_t model_inference(CNN& model, const cv::Mat& image){
    cv::Mat preprocessed_image;

    cv::resize(image, preprocessed_image, cv::Size(255, 255), 0, 0);
    cv::cvtColor(preprocessed_image, preprocessed_image, cv::COLOR_BGR2GRAY);
    preprocessed_image.convertTo(preprocessed_image, CV_32F, 1/255.0);

    cv::imshow("Image after preprocessing", preprocessed_image);
    cv::waitKey(0);

    torch::Tensor tensor = torch::from_blob(
        preprocessed_image.data, {1, preprocessed_image.rows, preprocessed_image.cols}, torch::kFloat32); // C, H, W

    tensor = tensor.unsqueeze(0); // B, C, H, W

    std::cout << "Input tensor size: " << tensor.sizes() << std::endl;

    torch::NoGradGuard no_grad;
    model->eval();

    torch::Tensor output = model->forward(tensor);
    int64_t pred = output.argmax(1).item<int64_t>();
    
    return pred;
}

int main() {
    CNN model = CNN();
    torch::save(model, "../models/cnn_init.pt");
    
    train_model(model);
    
    CNN model_infer;
    cv::Mat image = cv::imread("/home/anna/Pictures/litter.png");
    if (image.empty()){
        std::cerr << "Couldn't read the photo\n";
        return 1;
    }
    torch::load(model_infer, "../models/cnn_trained.pt");
    std::cout << "Model loaded for inference\n";

    int64_t pred = model_inference(model_infer, image);
    std::cout << "Predicted class: " << pred << std::endl;

    return 0;

}