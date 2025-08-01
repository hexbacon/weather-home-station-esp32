/**
 * @file rgb_led.c
 * @brief RGB LED Status Indicator Implementation for ESP32 Weather Station
 * @details This file implements RGB LED functionality for the ESP32 weather
 *          station project. It provides visual status indicators using PWM-
 *          controlled RGB LED to display system states including WiFi status,
 *          sensor operation, server status, and error conditions. The
 *          implementation uses ESP32's LEDC peripheral for smooth color
 *          transitions and precise brightness control.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#include <stdbool.h>

#include "driver/ledc.h"
#include "hal/ledc_types.h"
#include "rgb_led.h"
#include "freertos/FreeRTOS.h"
#include "tasks_common.h"
#include "esp_log.h"
// RGB LED Info array
ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

// Handle for RGB LED PMW Init
bool g_pmw_init_handle = false;

static const char TAG [] = "LED";

/**
 * @brief Togges the red LED
 * @param pvParameters parameter which can be passed to the task
 */
static void red_led_task(void *pvParameters)
{
    for (;;)
    {
        // Turn red on
        ledc_set_duty(
            ledc_ch[0].mode, 
            ledc_ch[0].channel, 
            RED_BLINK_ON_DUTY);
        ledc_set_duty(
            ledc_ch[1].mode, 
            ledc_ch[1].channel, 
            0);
        ledc_set_duty(
            ledc_ch[2].mode, 
            ledc_ch[2].channel, 
            0);
        ledc_update_duty(
            ledc_ch[0].mode, 
            ledc_ch[0].channel);
        ledc_update_duty(
            ledc_ch[1].mode, 
            ledc_ch[1].channel);
        ledc_update_duty(
            ledc_ch[2].mode, 
            ledc_ch[2].channel);
        ESP_LOGI(TAG, "LED: red light on");
            vTaskDelay(pdMS_TO_TICKS(RED_BLINK_DELAY_MS));

        // Turn red off
        ledc_set_duty(
            ledc_ch[0].mode, 
            ledc_ch[0].channel, 
            RED_BLINK_OFF_DUTY);
        ledc_update_duty(
            ledc_ch[0].mode, 
            ledc_ch[0].channel);
        ESP_LOGI(TAG, "LED: red light off");
        vTaskDelay(pdMS_TO_TICKS(RED_BLINK_DELAY_MS));
    }
}

/**
 * Initializes the RGB LED settings per channel, including
 * the GPIO for each color, mode and timer configuration
 */
static void rgb_led_pmw_init(void) 
{
	int rgb_ch;

	// Red
	ledc_ch[0].channel          = LEDC_CHANNEL_0;
    ledc_ch[0].gpio             = RGB_LED_RED_GPIO;
    ledc_ch[0].mode             = LEDC_HIGH_SPEED_MODE;
    ledc_ch[0].timer_index     = LEDC_TIMER_0;

    // Green
	ledc_ch[1].channel          = LEDC_CHANNEL_1;
    ledc_ch[1].gpio             = RGB_LED_GREEN_GPIO;
    ledc_ch[1].mode             = LEDC_HIGH_SPEED_MODE;
    ledc_ch[1].timer_index     = LEDC_TIMER_0;

    // Blue
	ledc_ch[2].channel          = LEDC_CHANNEL_2;
    ledc_ch[2].gpio             = RGB_LED_BLUE_GPIO;
    ledc_ch[2].mode             = LEDC_HIGH_SPEED_MODE;
    ledc_ch[2].timer_index     = LEDC_TIMER_0;

    // Configure timer 0
    ledc_timer_config_t ledc_timer =
    {
        .duty_resolution        = LEDC_TIMER_8_BIT,
        .freq_hz                = 100,
        .speed_mode             = LEDC_HIGH_SPEED_MODE,
        .timer_num              = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    // Configure channels
    for (rgb_ch = 0; rgb_ch < RGB_LED_CHANNEL_NUM; rgb_ch++)
    {
        ledc_channel_config_t ledc_channel =
        {
            .channel            = ledc_ch[rgb_ch].channel,
            .duty               = 0,
            .hpoint             = 0,
            .gpio_num           = ledc_ch[rgb_ch].gpio,
            .intr_type          = LEDC_INTR_DISABLE,
            .speed_mode         = ledc_ch[rgb_ch].mode,
        };
        ledc_channel_config(&ledc_channel);
    }
    g_pmw_init_handle = true;
}


/**
 * Sets the RGB color.
 */
static void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    // Value should be 0-255 for an 8-bit number
    ledc_set_duty(ledc_ch[0].mode, ledc_ch[0].channel, red);
    ledc_update_duty(ledc_ch[0].mode, ledc_ch[0].channel);

    ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, green);
    ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);

    ledc_set_duty(ledc_ch[2].mode, ledc_ch[2].channel, blue);
    ledc_update_duty(ledc_ch[2].mode, ledc_ch[2].channel);
}


void rgb_led_wifi_app_started(void)
{
    if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }
    
    rgb_led_set_color(255, 102, 255);
}


void rgb_led_http_server_started(void)
{
     if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }
    
    rgb_led_set_color(204 , 102, 51);
}


void rgb_led_wifi_connected(void)
{
    if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }
    
    rgb_led_set_color(0, 255, 153);
}

void rgb_led_dht11_started(void)
{
    if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }
    
    rgb_led_set_color(32, 66, 63);
}

void rgb_led_error(void)
{

    if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }
    xTaskCreatePinnedToCore(red_led_task, "red_led_task", DHT_SENSOR_TASK_STACK_SIZE, NULL, DHT_SENSOR_TASK_PRIORITY, NULL, DHT_SENSOR_TASK_CORE_ID);
}

void rgb_led_dht11_read()
{
    if (g_pmw_init_handle == false)
    {
        rgb_led_pmw_init();
    }

    rgb_led_set_color(147, 251, 255);
}