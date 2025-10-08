import cnn
import torch
import torch.optim as optim
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
import os


def train_model(model):
    model.train()
    batch_size = 64
    data_root = "../data/HG14/HG14-Hand-Gesture/"
    img_size = 32
    num_epochs = 5

    transform = transforms.Compose([
        transforms.Grayscale(num_output_channels=1),
        transforms.Resize((img_size, img_size)),
        transforms.ToTensor(),
        transforms.Normalize((0.5,), (0.5,))
    ])

    dataset = datasets.ImageFolder(root=os.path.join(data_root), transform=transform)
    train_size = int(0.8 * len(dataset))
    val_size = len(dataset) - train_size
    train_dataset, val_dataset = torch.utils.data.random_split(dataset, [train_size, val_size])

    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=1e-3)

    for epoch in range(num_epochs):
        epoch_loss = 0.0
        for batch_idx, (inputs, labels) in enumerate(train_loader):
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            epoch_loss += loss.item()
            if batch_idx % 25 == 0:
                print(f"Epoch {epoch} batch {batch_idx}/{len(train_loader)} Current loss: {loss.item()}")

        # Validation phase
        model.eval()
        with torch.no_grad():
            val_loss = 0.0
            for inputs, labels in val_loader:
                outputs = model(inputs)
                loss = criterion(outputs, labels)
                val_loss += loss.item()
            print(f"Epoch {epoch} Train loss: {epoch_loss / len(train_loader)} Val loss: {val_loss / len(val_loader)}")


if __name__ == "__main__":
    torch.manual_seed(42)
    model = cnn.CNN()
    train_model(model)
    os.makedirs("../models", exist_ok=True)
    torch.save(model.state_dict(), "../models/cnn_trained_input32.pt")
    print("Model saved to ../models/cnn_trained_input32.pt")