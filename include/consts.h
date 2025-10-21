/**
 * @file consts.h
 * @brief Contains constant definitions for GPIO pins, Wi-Fi credentials, and gesture names.
 *
 * This file centralizes various hardware and software constants used throughout
 * the ESP32 gesture recognition project, making them easy to configure and manage.
 */

#ifndef CONSTS_H
#define CONSTS_H

/**
 * @name Camera GPIO Pin Definitions
 * @brief GPIO pin assignments for the ESP32-CAM camera module.
 * @{ 
 */
#define PWDN_GPIO_NUM  32 ///< Power Down Pin
#define RESET_GPIO_NUM -1 ///< Reset Pin (set to -1 if not used)
#define XCLK_GPIO_NUM  0  ///< System Clock Pin
#define SIOD_GPIO_NUM  26 ///< SCCB Data Pin
#define SIOC_GPIO_NUM  27 ///< SCCB Clock Pin

#define Y9_GPIO_NUM    35 ///< D9 Data Pin
#define Y8_GPIO_NUM    34 ///< D8 Data Pin
#define Y7_GPIO_NUM    39 ///< D7 Data Pin
#define Y6_GPIO_NUM    36 ///< D6 Data Pin
#define Y5_GPIO_NUM    21 ///< D5 Data Pin
#define Y4_GPIO_NUM    19 ///< D4 Data Pin
#define Y3_GPIO_NUM    18 ///< D3 Data Pin
#define Y2_GPIO_NUM    5  ///< D2 Data Pin
#define VSYNC_GPIO_NUM 25 ///< VSYNC Pin
#define HREF_GPIO_NUM  23 ///< HREF Pin
#define PCLK_GPIO_NUM  22 ///< Pixel Clock Pin
/** @} */ // End of Camera GPIO Pin Definitions

/**
 * @brief GPIO pin number for the LED.
 *
 * This can be 4 for the flash LED or 33 for the back LED, depending on the ESP32-CAM board.
 */
#define LED_GPIO_NUM 4

/**
 * @brief Wi-Fi SSID for connecting to the network.
 *
 * TODO: Change from static to a more secure and configurable method (e.g., NVS).
 */
static const char* WIFI_SSID = "<your ssid";

/**
 * @brief Wi-Fi Password for connecting to the network.
 *
 * TODO: Change from static to a more secure and configurable method (e.g., NVS).
 */
static const char* WIFI_PASS = "<your password>";

/**
 * @brief Logging tag for ESP_LOGx macros.
 */
static const char* TAG = "gestures";

/**
 * @brief Array of gesture names corresponding to model output classes.
 */
static const char* GESTURES[] = {"fist", "1 finger", "2 fingers", "3 fingers", "4 fingers", "palm", "phone", "mouth", "open mouth", "ok", "pinky", "rock1", "rock2", "stop"};

#endif // CONSTS_H