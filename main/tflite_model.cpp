#include "tflite_model.h"
#include "esp_log.h"
#include "consts.h"
#include "esp_heap_caps.h"
  
TFLiteModel::TFLiteModel(const unsigned char* model_data, const unsigned int* model_size) 
    : model_data_(model_data), 
      model_size_(model_size),
      input_(nullptr),
      output_(nullptr),
      initialized_(false) {
}

bool TFLiteModel::init() {
    ESP_LOGI(TAG, "Model: Initializing...");

    if (!model_data_ || model_size_ == 0) {
        return false;
    }

    const tflite::Model* model = tflite::GetModel(model_data_);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        return false;
    }

    // Op resolver
    op_resolver_.AddConv2D();
    op_resolver_.AddDepthwiseConv2D();
    op_resolver_.AddRelu();
    op_resolver_.AddMaxPool2D();
    op_resolver_.AddReshape();
    op_resolver_.AddFullyConnected();
    op_resolver_.AddTranspose();  
    op_resolver_.AddQuantize();
    op_resolver_.AddDequantize();

    tensor_arena_ = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);

    // Allocate memory from the tensor_arena for the model's tensors
    interpreter_ = std::make_unique<tflite::MicroInterpreter>(
        model, op_resolver_, tensor_arena_, kTensorArenaSize);

    TfLiteStatus allocate_status = interpreter_->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        ESP_LOGE(TAG, "Model: AllocateTensors() failed");
        return false;
    }

    input_ = interpreter_->input(0);
    output_ = interpreter_->output(0);
    initialized_ = true;
   
    ESP_LOGI(TAG, "Model: Initialized successfully");

    return true;

}


TfLiteStatus TFLiteModel::invoke(){
    return interpreter_->Invoke();
}


TFLiteModel::~TFLiteModel(){
    if (tensor_arena_) heap_caps_free(tensor_arena_);
}


