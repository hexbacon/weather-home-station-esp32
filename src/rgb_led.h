/**
 * @file rgb_led.h
 * @brief RGB LED Status Indicator Header for ESP32 Weather Station
 * @details This header file defines the interface for RGB LED status indicators
 *          used in the ESP32 weather station project. It provides function
 *          prototypes, GPIO pin definitions, and LEDC configuration constants
 *          for controlling RGB LED visual feedback to indicate system status,
 *          WiFi connectivity, sensor operations, and error conditions using
 *          PWM-based color control.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 2.0
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB LED GPIO Pin Assignments
#define RGB_LED_RED_GPIO        21              ///< GPIO pin for red LED channel
#define RGB_LED_GREEN_GPIO      22              ///< GPIO pin for green LED channel  
#define RGB_LED_BLUE_GPIO       23              ///< GPIO pin for blue LED channel

// RGB LED Configuration Constants
#define RGB_LED_CHANNEL_NUM     3               ///< Total number of LED channels (R, G, B)

// LED Blink Pattern Configuration
#define RED_BLINK_ON_DUTY       255             ///< PWM duty cycle for LED on state (0-255)
#define RED_BLINK_OFF_DUTY      0               ///< PWM duty cycle for LED off state
#define RED_BLINK_DELAY_MS      1000            ///< Blink delay duration in milliseconds

/**
 * @brief LEDC channel configuration structure
 * 
 * Structure defining the configuration parameters for each LED channel
 * using the ESP32's LEDC (LED Controller) peripheral for PWM control.
 */
typedef struct {
    int channel;        ///< LEDC channel number (0-7)
    int gpio;          ///< GPIO pin number for this LED channel
    int mode;          ///< LEDC mode (high-speed or low-speed)
    int timer_index;   ///< LEDC timer index (0-3)
} ledc_info_t;

/**
 * @brief Indicate WiFi application startup
 * 
 * Sets RGB LED to a specific color pattern to visually indicate
 * that the WiFi application has started initialization.
 * 
 * @note Typically displays a blue color pattern
 * @note Non-blocking function, returns immediately
 */
void rgb_led_wifi_app_started(void);

/**
 * @brief Indicate HTTP server startup
 * 
 * Sets RGB LED to a specific color pattern to visually indicate
 * that the HTTP web server has started and is ready to accept connections.
 * 
 * @note Typically displays a cyan color pattern
 * @note Non-blocking function, returns immediately
 */
void rgb_led_http_server_started(void);

/**
 * @brief Indicate WiFi connection established
 * 
 * Sets RGB LED to a specific color pattern to visually indicate
 * that the ESP32 has successfully connected to a WiFi access point
 * and obtained an IP address.
 * 
 * @note Typically displays a green color pattern
 * @note Non-blocking function, returns immediately
 */
void rgb_led_wifi_connected(void);

/**
 * @brief Indicate DHT11 sensor initialization
 * 
 * Sets RGB LED to a specific color pattern to visually indicate
 * that the DHT11 temperature and humidity sensor has been initialized
 * and is ready for readings.
 * 
 * @note Typically displays a yellow color pattern
 * @note Non-blocking function, returns immediately
 */
void rgb_led_dht11_started(void);

/**
 * @brief Indicate system error condition
 * 
 * Sets RGB LED to an error pattern (typically red blinking) to visually
 * indicate that the ESP32 has encountered an error condition such as
 * sensor read failure, WiFi connection loss, or other system faults.
 * 
 * @note Displays a red blinking pattern for high visibility
 * @note May block briefly for blink timing
 * @warning This function is called during error conditions
 */
void rgb_led_error(void);

/**
 * @brief Indicate DHT11 sensor reading activity
 * 
 * Sets RGB LED to a specific color pattern to visually indicate
 * that a DHT11 sensor reading is in progress or has completed successfully.
 * 
 * @note Typically displays a brief white flash or steady white
 * @note Non-blocking function, returns immediately
 * @note Called during normal sensor operation cycles
 */
void rgb_led_dht11_read(void);

#endif /* MAIN_RGB_LED_H_ */