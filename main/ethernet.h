#include "esp_event.h"

/***
 * @brief Event handler for Ethernet events.
 * @param arg User-defined argument (not used).
 * @param event_base The base of the event.
 * @param event_id The ID of the event.
 * @param event_data Pointer to event-specific data (not used).
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

/**
 * @brief Register and initialize Ethernet.
 * This function sets up the Ethernet interface, including MAC and PHY
 * configurations, and starts the Ethernet driver.
 */
void register_ethernet(void);