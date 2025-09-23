#include <algorithm>
#include <opencv2/opencv.hpp>
#include "gesture_dataset.h"

GestureDataset::GestureDataset(const std::string& root, int image_size): image_size_(image_size){
    /***
     * Load dataset from root directory
     * Assuming each subdirectory in root corresponds to a class and contains .jpg images
     * Store (image_path, label) pairs in images_ vector
     */
    fs::path rootp(root);

    if(!fs::exists(rootp) || !fs::is_directory(rootp)){
        throw std::runtime_error("GestureDataset: Invalid root directory: " + root);
    }

    // Get class directories
    std::vector<fs::path> class_dirs;
    for(const auto& entry : fs::directory_iterator(rootp)){
        if(entry.is_directory()){
            class_dirs.push_back(entry.path());
        }
    }
    if(class_dirs.empty()){
        throw std::runtime_error("GestureDataset: No class directories in: " + root);
    }

    std::sort(class_dirs.begin(), class_dirs.end());

    // Get image paths for each class
    images_.clear();
    images_.reserve(class_dirs.size()*1000); // we have 1000 images per class

    int label = 0;
    for(const auto& class_dir : class_dirs){
        std::vector<fs::path> img_paths;

        for(const auto& f : fs::directory_iterator(class_dir)){
            if(f.is_regular_file() && f.path().extension() == ".jpg"){
                img_paths.push_back(f.path());
            }
        }
        if(img_paths.empty()){
            throw std::runtime_error("GestureDataset: No images in class directory: " + class_dir.string());
        }
        std::sort(img_paths.begin(), img_paths.end());

        for (const auto& img_path : img_paths){
            images_.emplace_back(img_path, label);
        }

        label++;
    }
}

torch::data::Example<> GestureDataset::get(size_t index){
    /***
     * Load image, preprocess and return as tensor along with label
     * Preprocessing: Resize to image_size_ x image_size_, convert to grayscale, normalize to [0,1]
     * Return: (image_tensor, label_tensor)
     */
    if(index >= images_.size()){
        throw std::out_of_range("GestureDataset: Index out of range");
    }

    const auto& [img_path, label] = images_[index];
    cv::Mat img = cv::imread(img_path.string());
    if(img.empty()){
        throw std::runtime_error("GestureDataset: Could not read image: " + img_path.string());
    }
    cv::resize(img, img, cv::Size(image_size_, image_size_));
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    img.convertTo(img, CV_32F, 1/255.0);    
    torch::Tensor img_tensor = torch::from_blob(img.data, {1, img.rows, img.cols}, torch::kFloat32); // C, H, W
    torch::Tensor label_tensor = torch::tensor(label, torch::kInt64);

    return {std::move(img_tensor), std::move(label_tensor)};
}

torch::optional<size_t> GestureDataset::size() const {
    return images_.size();
}
