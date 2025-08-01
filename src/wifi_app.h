/**
 * @file wifi_app.h
 * @brief WiFi Application Header for ESP32 Weather Station
 * @details This header file defines the interface for WiFi management
 *          functionality used in the ESP32 weather station project. It provides
 *          configuration constants, data structures, function prototypes, and
 *          message types for handling dual-mode WiFi operation with Access
 *          Point configuration and Station connectivity management including
 *          network discovery and automatic reconnection capabilities.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#ifndef MAIN_WIFI_APP_H_
#define MAIN_WIFI_APP_H_

#include "esp_netif.h"
#include "freertos/FreeRTOS.h"

// WiFi Access Point Configuration
#define WIFI_AP_SSID                "ESP32"             ///< Access Point network name (SSID)
#define WIFI_AP_PASSWORD            "password"          ///< Access Point password (min 8 chars for WPA2)
#define WIFI_AP_CHANNEL             1                   ///< WiFi channel number (1-13)
#define WIFI_AP_SSID_HIDDEN         0                   ///< AP visibility (0=visible, 1=hidden)
#define WIFI_AP_MAX_CONNECTIONS     5                   ///< Maximum concurrent client connections
#define WIFI_AP_BEACON_INTERVAL     100                 ///< Beacon interval in milliseconds (recommended: 100ms)
#define WIFI_AP_IP                  "192.168.0.1"       ///< Access Point IP address
#define WIFI_AP_GATEWAY             "192.168.0.1"       ///< Access Point gateway (same as IP for AP mode)
#define WIFI_AP_NETMASK             "255.255.255.0"     ///< Access Point subnet mask (corrected typo)
#define WIFI_AP_BANDWIDTH           WIFI_BW_HT20        ///< Channel bandwidth (20MHz standard, 40MHz optional)

// WiFi Station Configuration
#define WIFI_STA_POWER_SAVE         WIFI_PS_NONE        ///< Power save mode (NONE=always on, MIN/MAX=power saving)

// WiFi Standards and Limits
#define MAX_SSID_LENGTH             32                  ///< IEEE 802.11 standard maximum SSID length
#define MAX_PASSWORD_LENGTH         64                  ///< IEEE 802.11 standard maximum password length
#define MAX_CONNECTION_RETRIES      5                   ///< Maximum retry attempts on connection failure

/**
 * @brief WiFi network interface objects
 * 
 * Global network interface handles for Station (STA) and Access Point (AP) modes.
 * These are initialized during WiFi setup and used throughout the application.
 */
extern esp_netif_t* esp_netif_sta;  ///< Network interface handle for Station mode
extern esp_netif_t* esp_netif_ap;   ///< Network interface handle for Access Point mode

/**
 * @brief WiFi application message types
 * 
 * Message identifiers used for inter-task communication via FreeRTOS queues.
 * These messages coordinate actions between WiFi events and application responses.
 */
typedef enum wifi_app_message
{
    WIFI_APP_MSG_START_HTTP_SERVER = 0,         ///< Request to start HTTP web server
    WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,   ///< WiFi connection initiated from web interface
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP,          ///< Station mode connected and received IP address

} wifi_app_message_e;

/**
 * @brief WiFi application queue message structure
 * 
 * Structure for passing messages between tasks via FreeRTOS queues.
 * Can be extended with additional parameters as needed.
 */
typedef struct wifi_app_queue_message
{
    wifi_app_message_e msgID;   ///< Message identifier from wifi_app_message_e enum
} wifi_app_queue_message_t;

/**
 * @brief Send message to WiFi application task queue
 * 
 * Posts a message to the WiFi application task queue for processing.
 * This enables other tasks to trigger WiFi-related actions asynchronously.
 * 
 * @param msgID Message identifier from wifi_app_message_e enum
 * @return pdTRUE if message was successfully queued, pdFALSE if queue is full
 * 
 * @note This function is thread-safe and can be called from any task
 * @note Queue has finite capacity, function may fail if queue is full
 * @note Extend parameter list as needed for additional message data
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

/**
 * @brief Start the WiFi application
 * 
 * Initializes and starts the WiFi subsystem including:
 * 1. Creates WiFi application FreeRTOS task
 * 2. Initializes WiFi driver and network interfaces
 * 3. Configures Access Point mode with specified settings
 * 4. Sets up event handlers for WiFi and IP events
 * 5. Starts DHCP server for AP mode
 * 
 * @note Must be called after NVS initialization
 * @note Creates task on core 0 with priority 5
 * @note Automatically starts in AP mode, switches to STA when configured
 * @note Non-blocking function, WiFi initialization continues in background task
 */
void wifi_app_start(void);

#endif /* MAIN_WIFI_APP_H_ */