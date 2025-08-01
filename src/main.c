/**
 * @file main.c
 * @brief Main Application Entry Point for ESP32 Weather Station
 * @details This file contains the main application entry point for the ESP32-based
 *          weather home station. It initializes all system components including
 *          WiFi connectivity, DHT11 temperature/humidity sensor, SSD1306 OLED
 *          display, and RGB LED status indicators. The application continuously
 *          reads environmental data and provides both local display and web-based
 *          access to sensor readings.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#include "nvs_flash.h"
#include "wifi_app.h"
#include "DHT11.h"
#include "esp_log.h"
#include "rgb_led.h"

void app_main(void)
{
    // Initialize Non-Volatile Storage (required for WiFi configuration storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Start WiFi application (Access Point + Station mode capability)
    wifi_app_start();
    
    // Initialize DHT11 temperature and humidity sensor on GPIO 4
    dht11_t sensor;
    dht11_init(&sensor, 4);
    
    // Configure sensor reading interval (1 minute between readings)
    const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
    
    // Initial delay to allow system components to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Main sensor reading loop
    while (1) 
    {
        if (dht11_read(&sensor) == ESP_OK) 
        {
            // Log successful sensor reading
            ESP_LOGI("DHT11", "Temperature: %d°C, Humidity: %d%%",
                     dht11_get_temperature(&sensor),
                     dht11_get_humidity(&sensor));
        } else 
        {
            // Log sensor read failure and indicate error via LED
            ESP_LOGI("DHT11", "Failed to read from sensor");
            rgb_led_error();
        }
        
        // Wait for next reading cycle (DHT11 requires minimum 2 second intervals)
        vTaskDelay(xDelay);
    }
}