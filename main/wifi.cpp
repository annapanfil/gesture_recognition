#include "wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "nvs_flash.h"
#include "qrcode.h"
#include "json.hpp"


void WifiManager::initialize(const char* ap_ssid, const char* ap_pop)
{
    if(instance == nullptr)
    {
        instance = new WifiManager(ap_ssid, ap_pop);
        connected = false;
    }
    else
    {
        ESP_LOGW(TAG, "WifiManager already initialized");
    }
}

bool WifiManager::wait_for_connection(int timeout_ms) {
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        if (is_connected()) {
            return true;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        elapsed += 1000;
    }
    return false;
}


void WifiManager::print_qr()
{
    nlohmann::json qr_string = 
    {
        {"ver", "v1"},
        {"name", instance->ap_ssid},
        {"pop", instance->ap_pop},       
        {"transport", "softap"}
    };
    esp_qrcode_config_t qr = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&qr, qr_string.dump().c_str());
}

void WifiManager::wifi_prov_handler(void *user_data, wifi_prov_cb_event_t event, void *event_data)
{
    switch(event)
    {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "[WIFI_PROV_START]");
            break;
        case WIFI_PROV_CRED_RECV:
            ESP_LOGI(TAG, "cred: ssid: %s pass: %s",
                 ((wifi_sta_config_t*)event_data)->ssid, 
                 ((wifi_sta_config_t*)event_data)->password);
            break;
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Prov success");
            wifi_prov_mgr_stop_provisioning();
            ESP_LOGI(TAG, "Rebooting in 3 seconds...");
            vTaskDelay(pdMS_TO_TICKS(3000));
            esp_restart();
            break;
        case WIFI_PROV_CRED_FAIL:
            ESP_LOGE(TAG, "Wrong credentials");
            wifi_prov_mgr_reset_sm_state_on_failure(); // reset provisioning state machine
            print_qr();
            break;
        case WIFI_PROV_END:
            ESP_LOGI(TAG, "Prov ended");
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
    }
}

void WifiManager::wifi_event_handler(void* event_handler_arg,
                                esp_event_base_t event_base,
                                int32_t event_id,
                                void* event_data){
    static int retry_cnt = 0;
    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START: // triggered by esp_wifi_start
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGE(TAG, "Wifi disconnected, retrying...");
                retry_cnt++;
                if(retry_cnt < MAX_RETRY_NUM)
                {
                    esp_wifi_connect();
                }
                else
                {
                    ESP_LOGE(TAG, "Connection error");
                }
                break;
            default:
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG, "Station ip :" IPSTR, IP2STR(&event->ip_info.ip));
            connected = true;
            retry_cnt = 0;
        }
    }

}

void WifiManager::wifi_hw_init(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    // Handle wifi events
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    esp_netif_create_default_wifi_sta(); // as a station for main usage
    esp_netif_create_default_wifi_ap(); // as access point for wifi provisioning

    wifi_init_config_t cfg =  WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiManager::prov_start()
{
    // Initialise provisioning process
    wifi_prov_mgr_config_t cfg = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler  = {
            .event_cb = wifi_prov_handler,
            .user_data = NULL
        }
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(cfg));

    // Check for existing data
    bool is_provisioned = false;
    // wifi_prov_mgr_reset_provisioning(); // Uncomment to reset provisioning
    wifi_prov_mgr_is_provisioned(&is_provisioned);
    wifi_prov_mgr_disable_auto_stop(100);
    
    if(is_provisioned)
    {
        ESP_LOGI(TAG, "Already provisioned");
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_start();
    }
    else
    {
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, instance->ap_pop, instance->ap_ssid, NULL));
        print_qr();
    }


}

