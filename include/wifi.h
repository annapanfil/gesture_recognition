#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

/**
 * @brief Manages WiFi connectivity and provisioning.
 *
 * This class implements the Singleton pattern to provide a single point of
 * access for WiFi operations. It handles initializing the WiFi hardware,
 * starting the provisioning process (if needed), and managing the WiFi
 * connection.
 */
class WifiManager {
public:
    // Singleton pattern: delete copy constructor and assignment operator
    WifiManager(const WifiManager&) = delete; ///< Deleted copy constructor.
    WifiManager& operator=(const WifiManager&) = delete; ///< Deleted assignment operator.


    /**
     * @brief Initializes the WifiManager singleton instance with a custom SSID and
     * proof of possession (password).
     *
     * @param ap_ssid The SSID of the SoftAP.
     * @param ap_pop The proof of possession (password) for the SoftAP.
     */
    static void initialize(const char* ap_ssid = "PROV_ESP", const char* ap_pop = "abcd1234");

    /**
     * @brief Gets the singleton instance of the WifiManager.
     *
     * @return A reference to the WifiManager instance.
     */
    static WifiManager& getInstance(){
        return *instance;
    }

    /**
     * @brief Starts the WiFi provisioning process.
     */
    static void prov_start();

    /**
     * @brief Initializes the WiFi hardware.
     */
    static void wifi_hw_init();

    /**
     * @brief Destructor for the WifiManager.
     */
    ~WifiManager();

    /**
     * @brief Waits for a WiFi connection with a timeout.
     */
    static bool wait_for_connection(int timeout_ms = 30000);

    /**
     * @brief Checks if the WiFi is connected.
     */
    static bool is_connected() {return connected;};

private:
    /**
     * @brief Private constructor for the WifiManager.
     * @param ap_ssid The SSID of the SoftAP.
     * @param ap_pop The proof of possession (password) for the SoftAP.
     */
    WifiManager(const char* ap_ssid, const char* ap_pop): ap_ssid(ap_ssid), ap_pop(ap_pop) {};

    static inline WifiManager* instance = nullptr; ///< The singleton instance of the WifiManager.

    static const int MAX_RETRY_NUM = 5; ///< The maximum number of times to retry connecting to the WiFi network.
    static inline const char* TAG = "wifi_mgr"; ///< The logging tag for the WifiManager.
    static inline bool connected = false; ///< Flag indicating if WiFi is connected.
    const char* ap_ssid; ///< The SSID of the SoftAP.
    const char* ap_pop;  ///< Proof of possession (password) for the SoftAP.

    /**
     * @brief Prints a QR code for WiFi provisioning.
     */
    static void print_qr();

    /**
     * @brief Handles WiFi provisioning events.
     */
    static void wifi_prov_handler(void* user_data, wifi_prov_cb_event_t event,
                                  void* event_data);

    /**
     * @brief Handles WiFi events.
     */
    static void wifi_event_handler(void* event_handler_arg,
                                   esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
};

#endif  // WIFI_H