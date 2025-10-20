# 🤘 ESP32 Gesture Recognition

**Real-time hand gesture recognition** on the **ESP32-CAM** using **TensorFlow Lite**. Includes **Web Interface** to see the camera feed and the recognised gesture.


## ⚡ Installation & Build

### What You'll Need

* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) - The official development framework for ESP32.
* ESP32-CAM board
* Micro USB cable
* QEMU (optional, for local simulation)  

### Building and Flashing

1.  **Clone the repo:**
    ```bash
    git clone git@github.com:annapanfil/gesture_recognition.git
    cd gesture_recognition
    ```

2.  **Set up your Wi-Fi:**

    Edit `include/const.h` with your Wi-Fi credentials:
    ```cpp
    strcpy((char*)wifi_config.sta.ssid, "YOUR_WIFI_SSID");
    strcpy((char*)wifi_config.sta.password, "YOUR_WIFI_PASSWORD");
    ```
3.  **Build, Flash, and Go!**

    It will install all the prerequisites (from CMake and idf) automatically if you don't have them.
    ```bash
    idf.py build flash
    ```

4.  **See it running:**
    ```bash
    idf.py monitor
    ```
5. **Use the web interface:**
    1.  Make sure your computer is on the same Wi-Fi network as your ESP32.
    2.  Open your web browser and go to the IP address of your ESP32 (check the serial monitor for the address).
    3.  You'll see a simple web page. Click the "Capture & Detect" button and show it your best gestures!

    <!-- TODO: web gui screenshot -->

## Gestures
The model recognises 14 gestures:

![Recognised gestures](schemas/gestures.jpg)

## 🔧 Insides 

📂 Project Structure
```
gesture_recognition/
├── include/        # Header files
├── main/           # Main source files
├── models/         # Model file (trained, quantised and exported to tflite)
├── scripts/        # Python scripts for model training and conversion
├── CMakeLists.txt
└── partitions.csv  # Custom partitioning
```

## CNN Model
The CNN model processes grayscale images of size 32×32 pixels. It consists of two convolutional layers with channel progression 1 → 8 → 16, each followed by batch normalization, ReLU activation, and max pooling. The feature maps are then flattened and passed through a fully connected layer, which outputs the predicted gesture class.
![nn schema](schemas/nn.png)

If you want to train your own model, the training script is ready in `scripts/train.py`.

## Data flow

Camera → Preprocess (normalise and resize) → TensorFlow Lite → Web UI

## 🚀 Performance
- Inference time: ...
- Memory usage: ...
- Accuracy: ...
- Latency: ...

## Memory management

Model and tensor arena are kept in **PSRAM**, because of their size (250KB and 30KB).

**Stack size** for main task was increased to 16KB for model, camera and wi-fi initialisation.

### Custom Flash memory partitioning
`NVS`: Reduced from 24KB → 16KB (needed only for WiFi credential)</br>
`phy_init`: Kept at 4KB (ESP32 requirement)</br>
`factory`: Increased from 1MB → 1.6MB (to fit TensorFlow model + application code)</br>
Free: 0.4MB



<!-- TODO  shemat jak komponenty współpracują (ESP, web GUI, dioda, mosquito) -->

---
© Anna Panfil 2025