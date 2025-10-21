"""Script to convert a PyTorch CNN model to TensorFlow Lite format.

This script loads a pre-trained PyTorch CNN model, converts it into a
TensorFlow Lite model, and then exports it to a `.tflite` file. It also
includes a verification step to compare the inference results of the
original PyTorch model and the converted TensorFlow Lite model.
"""
import torch
import ai_edge_torch
import tensorflow as tf
import numpy
import cnn

if __name__ == "__main__":
    model = cnn.CNN()
    model.load_state_dict(torch.load("../models/cnn_trained_input32.pt"))

    dummy_input = (torch.randn(1, 1, 32, 32),)  # 1 batch, 1x32x32

    tfl_converter_flags = {'optimizations': [tf.lite.Optimize.DEFAULT]}

    edge_model = ai_edge_torch.convert(model.eval(), dummy_input) # _ai_edge_converter_flags=tfl_converter_flags
    edge_model.export("../models/model_nof16.tflite")

    print("Model converted to TFLite format and saved to ../models/model_nof16.tflite")

    # Check the model
    torch_output = model(*dummy_input)
    edge_output = edge_model(*dummy_input)

    if (numpy.allclose(
        torch_output.detach().numpy(),
        edge_output,
        atol=1e-5,
        rtol=1e-5,
    )):
        print("Inference result with Pytorch and TfLite was within tolerance")
    else:
        print("Something wrong with Pytorch --> TfLite")