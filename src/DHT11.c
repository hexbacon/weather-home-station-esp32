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
 * @version 2.0
 */

#include "DHT11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rgb_led.h"

#define DHT_MAX_TIMINGS 85
#define DHT_READ_RETRIES 3

const char TAG[] = "DHT11";

void dht11_init(dht11_t *sensor, int gpio_num)
{
    sensor->gpio_num = gpio_num;
    sensor->temperature = 0;
    sensor->humidity = 0;

    gpio_reset_pin(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(gpio_num, 1);

    ESP_LOGI(TAG, "dht11_init: init complete");
    rgb_led_dht11_started();
}

static void dht11_set_output(dht11_t *sensor)
{
    gpio_set_direction(sensor->gpio_num, GPIO_MODE_OUTPUT_OD);
}

static void dht11_set_input(dht11_t *sensor)
{
    gpio_set_direction(sensor->gpio_num, GPIO_MODE_INPUT);
}

esp_err_t dht11_read_once(dht11_t *sensor)
{
    int data[5] = {0};
    int bit_idx = 0, byte_idx = 0;

    // Send start signal
    dht11_set_output(sensor);
    gpio_set_level(sensor->gpio_num, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // >18ms to start
    gpio_set_level(sensor->gpio_num, 1);
    esp_rom_delay_us(30);

    // Wait for sensor response
    dht11_set_input(sensor);

    int64_t timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 1)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for LOW");
            return ESP_ERR_TIMEOUT;
        }
    }

    timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 0)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for HIGH");
            return ESP_ERR_TIMEOUT;
        }
    }

    timeout = esp_timer_get_time();
    while (gpio_get_level(sensor->gpio_num) == 1)
    {
        if (esp_timer_get_time() - timeout > 100)
        {
            ESP_LOGI(TAG, "dht11_read: TIMEOUT second LOW");
            return ESP_ERR_TIMEOUT;
        }
    }

    // Read 40 bits (5 bytes)
    for (int i = 0; i < 40; i++)
    {
        // Wait for LOW pulse
        timeout = esp_timer_get_time();
        while (gpio_get_level(sensor->gpio_num) == 0)
        {
            if (esp_timer_get_time() - timeout > 100)
            {
                ESP_LOGI(TAG, "dht11_read: TIMEOUT waiting for bit HIGH");
                return ESP_ERR_TIMEOUT;
            }
        }

        // Measure HIGH pulse width
        int64_t start = esp_timer_get_time();
        while (gpio_get_level(sensor->gpio_num) == 1)
        {
            if (esp_timer_get_time() - start > 100)
                break;
        }
        int pulse_width = (int)(esp_timer_get_time() - start);

        // Store bit based on width
        data[byte_idx] <<= 1;
        if (pulse_width > 40)
        {
            data[byte_idx] |= 1;
        }

        bit_idx++;
        if (bit_idx == 8)
        {
            bit_idx = 0;
            byte_idx++;
        }
    }

    // Checksum validation
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != (checksum & 0xFF))
    {
        ESP_LOGI(TAG, "Checksum mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    sensor->humidity = data[0];
    sensor->temperature = data[2];

    return ESP_OK;
}

esp_err_t dht11_read(dht11_t *sensor)
{
    esp_err_t result = ESP_FAIL;
    for (int i = 0; i < DHT_READ_RETRIES; i++)
    {
        result = dht11_read_once(sensor);
        if (result == ESP_OK)
        {
            rgb_led_dht11_read();
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // short delay between retries
    }
    rgb_led_error();
    ESP_LOGI(TAG, "Failed to read from sensor");
    return result;
}

int dht11_get_temperature(dht11_t *sensor)
{
    return sensor->temperature;
}

int dht11_get_humidity(dht11_t *sensor)
{
    return sensor->humidity;
}

float dht11_celsius_to_fahrenheit(int celsius)
{
    return (celsius * 9.0f / 5.0f) + 32.0f;
}
