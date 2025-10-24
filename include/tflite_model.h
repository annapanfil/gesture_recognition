#pragma once
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_log.h"

/**
 * @brief A wrapper class for managing TensorFlow Lite Micro models.
 *
 * This class handles the initialization, inference, and memory management
 * for a TensorFlow Lite Micro model. It provides methods to access the
 * model's input and output tensors and to invoke the interpreter.
 */
class TFLiteModel {
public:
    /**
     * @brief Constructs a TFLiteModel object.
     *
     * @param model_data Pointer to the TensorFlow Lite model data.
     * @param model_size Pointer to the size of the model data.
     */
    TFLiteModel(const unsigned char* model_data, const unsigned int* model_size);

    /**
     * @brief Initializes the TensorFlow Lite Micro interpreter.
     *
     * This method allocates tensors and prepares the interpreter for inference.
     * @return True if initialization is successful, false otherwise.
     */
    bool init();

    /**
     * @brief Returns a pointer to the input tensor of the model.
     *
     * @return TfLiteTensor* Pointer to the input tensor.
     */
    TfLiteTensor* input() { return input_; }

    /**
     * @brief Returns a pointer to the output tensor of the model.
     *
     * @return TfLiteTensor* Pointer to the output tensor.
     */
    TfLiteTensor* output() { return output_; }

    /**
     * @brief Invokes the TensorFlow Lite Micro interpreter to perform inference.
     *
     * @return TfLiteStatus The status of the invocation (kTfLiteOk on success).
     */
    TfLiteStatus invoke();

    /**
     * @brief Checks if the model has been successfully initialized.
     *
     * @return True if the model is initialized, false otherwise.
     */
    bool is_initialized() const { return initialized_; }

    /**
     * @brief Sets the index of the last detected gesture.
     *
     * @param index The index of the last detected gesture.
     */
    void set_last_detected_index(int index) { last_detected_index_ = index; }

    /**
     * @brief Returns the index of the last detected gesture.
     *
     * @return int The index of the last detected gesture.
     */
    int last_detected_index() const { return last_detected_index_; }

    /**
     * @brief Destroys the TFLiteModel object and frees allocated resources.
     */
    ~TFLiteModel();

private:
    static inline const char* TAG = "model"; ///< Tag for logging.
    int last_detected_index_ = -1; ///< Index of the last detected gesture.

    static constexpr int kTensorArenaSize = 37 * 1024; ///< Size of the tensor arena in bytes.
    uint8_t* tensor_arena_; ///< Pointer to the tensor arena memory.

    const unsigned char* model_data_; ///< Pointer to the raw model data.
    const unsigned int* model_size_; ///< Pointer to the size of the raw model data.
    
    // tflite::MicroErrorReporter error_reporter_; ///< Error reporter for logging (currently commented out).
    tflite::MicroMutableOpResolver<9> op_resolver_; ///< Operator resolver for TensorFlow Lite Micro.
    std::unique_ptr<tflite::MicroInterpreter> interpreter_; ///< Unique pointer to the TensorFlow Lite Micro interpreter.
    
    TfLiteTensor* input_; ///< Pointer to the input tensor.
    TfLiteTensor* output_; ///< Pointer to the output tensor.
    bool initialized_ = false; ///< Flag indicating whether the model has been initialized.
};