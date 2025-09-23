#pragma once
#include <filesystem>
#include <vector>
#include <torch/torch.h>

namespace fs = std::filesystem;

class GestureDataset : public torch::data::datasets::Dataset<GestureDataset> {
public:
    GestureDataset(const std::string& root, int image_size = 256);
    torch::data::Example<> get(size_t index) override;
    torch::optional<size_t> size() const override;
private:
    int image_size_;
    std::vector<std::pair<fs::path, int>> images_;
};