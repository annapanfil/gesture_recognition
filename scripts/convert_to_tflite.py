import torch
import ai_edge_torch
import tensorflow as tf
import numpy
import cnn

model = cnn.CNN()
model.load_state_dict(torch.load("../models/cnn_trained.pt"))

dummy_input = (torch.randn(1, 1, 64, 64),)  # 1 batch, 1x64x64

tfl_converter_flags = {'optimizations': [tf.lite.Optimize.DEFAULT], 'target_spec.supported_types': [tf.float16]}

edge_model = ai_edge_torch.convert(model.eval(), dummy_input) # _ai_edge_converter_flags=tfl_converter_flags
edge_model.export("../models/model.tflite")

print("Model converted to TFLite format and saved to ../models/model.tflite")

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