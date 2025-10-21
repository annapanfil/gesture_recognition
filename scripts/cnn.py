from torch import nn

class CNN (nn.Module):
    """A Convolutional Neural Network (CNN) model for gesture recognition.

    The model consists of two convolutional blocks followed by a fully connected layer.
    It is designed to process grayscale images of gestures.
    """
    def __init__(self):
        """Initializes the CNN model layers.

        The architecture includes:
        - Two convolutional layers with Batch Normalization, ReLU activation, and Max Pooling.
        - A Flatten layer to convert 2D feature maps to a 1D vector.
        - A Dropout layer for regularization.
        - A final Fully Connected layer for classification into 14 classes.
        """
        super(CNN, self).__init__()
        self.conv1 = nn.Conv2d(1, 8, kernel_size=3) # 32x32x1 -> 30x30x8
        self.bn1 = nn.BatchNorm2d(8)
        self.relu1 = nn.ReLU()
        self.pool1 = nn.MaxPool2d(kernel_size=2) # 15x15x8

        self.conv2 = nn.Conv2d(8, 16, kernel_size=3) # 13x13x16
        self.bn2 = nn.BatchNorm2d(16)
        self.relu2 = nn.ReLU()
        self.pool2 = nn.MaxPool2d(kernel_size=2) # 6x6x16

        self.flatten = nn.Flatten()
        self.dropout = nn.Dropout(0.5)
        self.fc1 = nn.Linear(6*6*16, 14) # 14 classes

    def forward(self, x):
        """Defines the forward pass of the CNN model.

        Args:
            x (torch.Tensor): The input tensor, expected to be a grayscale image batch.

        Returns:
            torch.Tensor: The output tensor containing the logits for each class.
        """
        x = self.conv1(x)
        x = self.pool1(self.relu1(self.bn1(x)))
        x = self.pool2(self.relu2(self.bn2(self.conv2(x))))
        x = self.flatten(x)
        x = self.dropout(x)
        x = self.fc1(x)
        return x