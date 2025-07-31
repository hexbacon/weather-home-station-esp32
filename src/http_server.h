/**
 * @file http_server.h
 * @brief HTTP Server Header for ESP32 Weather Station
 * @details This header file defines the interface for the HTTP web server
 *          functionality used in the ESP32 weather station project. It provides
 *          function prototypes, message types, and data structures for handling
 *          web requests, OTA updates, WebSocket communications, and inter-task
 *          messaging between the HTTP server and other system components.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 2.0
 */

#ifndef MAIN_HTTP_SERVER_H_
#define MAIN_HTTP_SERVER_H_

#include "freertos/FreeRTOS.h"

// OTA Update Status Constants
#define OTA_UPDATE_PENDING      0               ///< OTA update is in progress
#define OTA_UPDATE_SUCCESSFUL   1               ///< OTA update completed successfully
#define OTA_UPDATE_FAILED       -1              ///< OTA update failed

/**
 * @brief HTTP server message types for inter-task communication
 * 
 * These message IDs are used to communicate status updates between
 * the HTTP server and other application components via FreeRTOS queues.
 */
typedef enum http_server_message
{
    HTTP_MSG_WIFI_CONNECT_INIT = 0,     ///< WiFi connection initialization started
    HTTP_MSG_WIFI_CONNECT_SUCCESS,      ///< WiFi connection established successfully
    HTTP_MSG_WIFI_CONNECT_FAIL,         ///< WiFi connection attempt failed
    HTTP_MSG_OTA_UPDATE_SUCCESSFUL,     ///< Over-the-air update completed successfully
    HTTP_MSG_OTA_UPDATE_FAILED,         ///< Over-the-air update failed
} http_server_message_e;

/**
 * @brief HTTP server queue message structure
 * 
 * Structure used for passing messages through FreeRTOS queues between
 * the HTTP server and monitoring tasks.
 */
typedef struct http_server_queue_message
{
    http_server_message_e msgID;        ///< Message identifier from http_server_message_e enum
} http_server_queue_message_t;

/**
 * @brief Send a message to the HTTP server monitor queue
 * 
 * Posts a message to the HTTP server monitor task queue for processing.
 * This function is thread-safe and can be called from any task or ISR.
 * 
 * @param msgID Message identifier from the http_server_message_e enum
 * @return pdTRUE if message was successfully sent to queue, pdFALSE otherwise
 * 
 * @note The queue has a finite size, so this function may fail if queue is full
 * @note Expand parameters based on application requirements for additional data
 */
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

/**
 * @brief Start the HTTP web server
 * 
 * Initializes and starts the HTTP server on the configured port. Creates
 * the necessary tasks for handling HTTP requests and sets up URI handlers
 * for web interface, API endpoints, and OTA updates.
 * 
 * @note Must be called after WiFi initialization
 * @note Server runs on port 32768 by default
 * @note Creates HTTP server task with priority 4
 */
void http_server_start(void);

/**
 * @brief Stop the HTTP web server
 * 
 * Gracefully shuts down the HTTP server, closes all active connections,
 * and cleans up allocated resources. Also stops the associated monitor task.
 * 
 * @note This function is blocking until server is fully stopped
 * @note All active HTTP connections will be terminated
 */
void http_server_stop(void);

/**
 * @brief Timer callback for firmware update reset
 * 
 * Callback function executed by a timer after successful OTA firmware update.
 * Triggers a system restart to boot into the new firmware image.
 * 
 * @param args Timer callback arguments (unused, can be NULL)
 * 
 * @note This callback is executed in timer context, keep processing minimal
 * @note System will restart approximately 2 seconds after successful OTA update
 * @warning This function calls esp_restart() which does not return
 */
void http_server_fw_update_reset_callback(void* args);

#endif /* MAIN_HTTP_SERVER_H_ */