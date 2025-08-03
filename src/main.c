/**
 * @file main.c
 * @brief Main Application Entry Point for ESP32 Weather Station
 * @details This file contains the main application entry point for the ESP32-based
 *          weather home station. It initializes all system components including
 *          WiFi connectivity, DHT11 temperature/humidity sensor, I2C LCD display,
 *          and RGB LED status indicators. The application continuously reads 
 *          environmental data and provides both local LCD display and web-based
 *          access to sensor readings with dual temperature unit support.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 3, 2025 at 2:00 PM
 */

#include "nvs_flash.h"
#include "wifi_app.h"
#include "DHT11.h"
#include "esp_log.h"
#include "rgb_led.h"
#include "LiquidCrystal_I2C.h"
#include <stdbool.h>

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

    // Initialize I2C LCD display (16x2 at address 0x27)
    if (liquid_crystal_i2c_init(0x27, 16, 2) == ESP_OK) 
    {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Weather Station");
        lcd_set_cursor(0, 1);
        lcd_print("Initializing...");
        ESP_LOGI("MAIN", "LCD initialized successfully");
    } 
    else 
    {
        ESP_LOGE("MAIN", "Failed to initialize LCD");
    }

    // Initialize DHT11 temperature and humidity sensor on GPIO 4
    dht11_t sensor;
    dht11_init(&sensor, 4);

    // Configure sensor reading interval (1 minute between readings)
    const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;

    // Initial delay to allow system components to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Temperature unit toggle (true = Fahrenheit, false = Celsius)
    bool temp_fahrenheit = true;
    
    // Clear LCD and show ready message
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Weather Station");
    lcd_set_cursor(0, 1);
    lcd_print("Ready!");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Main sensor reading loop
    while (1)
    {
        if (dht11_read(&sensor) == ESP_OK)
        {
            // Get sensor readings
            int temperature = dht11_get_temperature(&sensor, temp_fahrenheit);
            int humidity = dht11_get_humidity(&sensor);
            
            // Format temperature unit string
            char temp_unit[8];
            sprintf(temp_unit, temp_fahrenheit ? "F" : "C");
            
            // Display on LCD
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Temp: ");
            lcd_print_int(temperature);
            lcd_print(temp_unit);
            
            lcd_set_cursor(0, 1);
            lcd_print("Humidity: ");
            lcd_print_int(humidity);
            lcd_print("%");
            
            // Log successful sensor reading
            ESP_LOGI("DHT11", "Temperature: %d%s, Humidity: %d%%",
                     temperature, temp_unit, humidity);
        }
        else
        {
            // Display error on LCD
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Sensor Error!");
            lcd_set_cursor(0, 1);
            lcd_print("Check DHT11");
            
            // Log sensor read failure and indicate error via LED
            ESP_LOGI("DHT11", "Failed to read from sensor");
            rgb_led_error();
        }

        // Wait for next reading cycle (DHT11 requires minimum 2 second intervals)
        vTaskDelay(xDelay);
    }
}