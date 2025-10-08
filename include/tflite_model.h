#pragma once
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_log.h"

class TFLiteModel {
public:
    TFLiteModel(const unsigned char* model_data, const unsigned int* model_size);
    bool init();
    // int forward(const float* input_data);
    TfLiteTensor* input() { return input_; }
    TfLiteTensor* output() { return output_; }
    bool is_initialized() const { return initialized_; }
    ~TFLiteModel();

private:
    static constexpr int kTensorArenaSize = 37 * 1024;
    uint8_t* tensor_arena_;

    const unsigned char* model_data_;
    const unsigned int* model_size_;
    
    // tflite::MicroErrorReporter error_reporter_;
    tflite::MicroMutableOpResolver<9> op_resolver_;
    std::unique_ptr<tflite::MicroInterpreter> interpreter_;
    
    TfLiteTensor* input_;
    TfLiteTensor* output_;
    bool initialized_ = false;
};

