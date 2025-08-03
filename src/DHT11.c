/**
 * @file DHT11.c
 * @brief DHT11 Temperature and Humidity Sensor Driver Implementation
 * @details This file implements the DHT11 sensor driver for the ESP32 weather
 *          station project. It provides complete functionality for reading
 *          temperature and humidity data from the DHT11 sensor using single-wire
 *          communication protocol. The implementation includes timing-critical
 *          operations, checksum validation, retry logic, and temperature unit
 *          conversion utilities for robust environmental data acquisition.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#include "DHT11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rgb_led.h"
#include <stdbool.h>

// DHT11 protocol constants
#define DHT_MAX_TIMINGS 85          // Maximum expected timing pulses
#define DHT_READ_RETRIES 3          // Number of retry attempts on read failure

const char TAG[] = "DHT11";

void dht11_init(dht11_t *sensor, int gpio_num)
{
    // Initialize sensor structure with GPIO pin and default values
    sensor->gpio_num = gpio_num;
    sensor->temperature = 0;
    sensor->humidity = 0;

    // Configure GPIO pin for open-drain operation (allows bidirectional communication)
    gpio_reset_pin(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(gpio_num, 1);    // Default to HIGH state (idle)

    ESP_LOGI(TAG, "dht11_init: init complete");
    rgb_led_dht11_started();        // Visual indication of sensor initialization
}

static void dht11_set_output(dht11_t *sensor)
{
    // Switch GPIO to output mode for sending start signal
    gpio_set_direction(sensor->gpio_num, GPIO_MODE_OUTPUT_OD);
}

static void dht11_set_input(dht11_t *sensor)
{
    // Switch GPIO to input mode for reading sensor response
    gpio_set_direction(sensor->gpio_num, GPIO_MODE_INPUT);
}

esp_err_t dht11_read_once(dht11_t *sensor)
{
    int data[5] = {0};              // Storage for 5 bytes of sensor data
    int bit_idx = 0, byte_idx = 0;  // Bit and byte counters for data parsing

    // Send start signal to DHT11 sensor
    dht11_set_output(sensor);
    gpio_set_level(sensor->gpio_num, 0);    // Pull line LOW for 20ms
    vTaskDelay(pdMS_TO_TICKS(20));          // DHT11 requires >18ms LOW signal
    gpio_set_level(sensor->gpio_num, 1);    // Release line (pull-up takes it HIGH)
    esp_rom_delay_us(30);                   // Wait 30μs before switching to input

    // Switch to input mode to read sensor response
    dht11_set_input(sensor);

    // Wait for sensor to pull line LOW (acknowledgment of start signal)
    int64_t timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 1)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for LOW");
            return ESP_ERR_TIMEOUT;
        }
    }

    // Wait for sensor to pull line HIGH (start of response signal)
    timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 0)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for HIGH");
            return ESP_ERR_TIMEOUT;
        }
    }

    // Wait for sensor to pull line LOW again (end of response signal)
    timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 1)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT second LOW");
            return ESP_ERR_TIMEOUT;
        }
    }

    // Read 40 bits of data (5 bytes: humidity_int, humidity_dec, temp_int, temp_dec, checksum)
    for (int i = 0; i < 40; i++)
    {
        // Wait for the LOW pulse to end (start of data bit)
        timeout = esp_timer_get_time();
        while (gpio_get_level(sensor->gpio_num) == 0)
        {
            if (esp_timer_get_time() - timeout > 100)
            {
                ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for bit HIGH");
                return ESP_ERR_TIMEOUT;
            }
        }

        // Measure the HIGH pulse width to determine bit value
        int64_t start = esp_timer_get_time();
        while (gpio_get_level(sensor->gpio_num) == 1)
        {
            if (esp_timer_get_time() - start > 100)
                break;  // Prevent infinite loop on stuck HIGH
        }
        int pulse_width = (int)(esp_timer_get_time() - start);

        // Decode bit based on pulse width: ~30μs = 0, ~70μs = 1
        data[byte_idx] <<= 1;           // Shift existing bits left
        if (pulse_width > 40)           // Threshold between 0 and 1 bit timing
        {
            data[byte_idx] |= 1;        // Set LSB to 1 for long pulse
        }
        // Short pulse leaves LSB as 0 (already shifted in)

        // Track bit position within current byte
        bit_idx++;
        if (bit_idx == 8)               // Complete byte received
        {
            bit_idx = 0;                // Reset bit counter
            byte_idx++;                 // Move to next byte
        }
    }

    // Validate data integrity using checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];   // Sum of first 4 bytes
    if (data[4] != (checksum & 0xFF))                           // Compare with received checksum
    {
        ESP_LOGI(TAG, "Checksum mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    // Store valid readings (DHT11 only uses integer parts)
    sensor->humidity = data[0];         // Humidity integer part (data[1] is always 0)
    sensor->temperature = data[2];      // Temperature integer part (data[3] is always 0)

    return ESP_OK;
}

esp_err_t dht11_read(dht11_t *sensor)
{
    esp_err_t result = ESP_FAIL;
    
    // Attempt to read sensor data with automatic retry on failure
    for (int i = 0; i < DHT_READ_RETRIES; i++)
    {
        result = dht11_read_once(sensor);
        if (result == ESP_OK)
        {
            rgb_led_dht11_read();       // Visual indication of successful read
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Short delay between retry attempts
    }
    
    // All retry attempts failed
    rgb_led_error();                    // Visual indication of read failure
    ESP_LOGI(TAG, "Failed to read from sensor");
    return result;
}

int dht11_get_temperature(dht11_t *sensor, bool fahrenheit)
{
    // Return cached temperature value from last successful read
    if (fahrenheit)
        return dht11_celsius_to_fahrenheit(sensor->temperature);
    return sensor->temperature;
}

int dht11_get_humidity(dht11_t *sensor)
{
    // Return cached humidity value from last successful read
    return sensor->humidity;
}

float dht11_celsius_to_fahrenheit(int celsius)
{
    // Standard temperature conversion formula: F = (C * 9/5) + 32
    return (celsius * 9.0f / 5.0f) + 32.0f;
}
