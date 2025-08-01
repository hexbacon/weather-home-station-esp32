/**
 * @file DHT11.h
 * @brief DHT11 Temperature and Humidity Sensor Driver Header
 * @details This header file defines the interface for the DHT11 temperature and
 *          humidity sensor driver used in the ESP32 weather station project.
 *          It provides data structures, function prototypes, and configuration
 *          constants for reading environmental data from the DHT11 sensor using
 *          single-wire communication protocol with proper timing and error
 *          handling capabilities.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#ifndef MAIN_DHT11_H_
#define MAIN_DHT11_H_

#include "esp_err.h"

// DHT11 Sensor Configuration Constants
#define DHT11_GPIO_SENSOR_PIN               4           ///< Default GPIO pin for DHT11 sensor data line
#define DHT11_START_SIGNAL_LOW_MS           20          ///< Duration to pull data line low for start signal (ms)
#define DHT11_TIMEOUT                       100         ///< General timeout for DHT11 operations (ms)
#define DHT11_START_SIGNAL_TIMEOUT_US       2000        ///< Timeout for start signal response (microseconds)

/**
 * @brief DHT11 sensor data structure
 * 
 * This structure holds the configuration and last read values from the DHT11 sensor.
 * The temperature is stored in Celsius and humidity as a percentage.
 */
typedef struct dht11
{
    int gpio_num;       ///< GPIO pin number connected to DHT11 data line
    int temperature;    ///< Last read temperature value in Celsius
    int humidity;       ///< Last read humidity value in percentage (0-100%)
} dht11_t;

/**
 * @brief Initialize the DHT11 sensor driver
 * 
 * Configures the specified GPIO pin for DHT11 communication and initializes
 * the sensor structure with default values.
 * 
 * @param sensor Pointer to DHT11 sensor structure to initialize
 * @param gpio_num GPIO pin number connected to DHT11 data line
 * 
 * @note The GPIO pin will be configured as open-drain output with pull-up
 * @note Initial temperature and humidity values are set to 0
 */
void dht11_init(dht11_t *sensor, int gpio_num);

/**
 * @brief Read temperature and humidity from DHT11 sensor
 * 
 * Performs a complete read cycle from the DHT11 sensor including:
 * 1. Sending start signal to sensor
 * 2. Waiting for sensor response
 * 3. Reading 40 bits of data (humidity + temperature + checksum)
 * 4. Validating checksum
 * 5. Updating sensor structure with new values
 * 
 * @param sensor Pointer to initialized DHT11 sensor structure
 * @return ESP_OK on successful read, ESP_FAIL on communication error or checksum failure
 * 
 * @note This function is blocking and takes approximately 20-30ms to complete
 * @note Temperature range: 0-50°C, Humidity range: 20-90% RH
 * @warning Do not call this function more frequently than once every 2 seconds
 */
esp_err_t dht11_read(dht11_t *sensor);

/**
 * @brief Get the last read temperature value
 * 
 * Returns the temperature value from the most recent successful sensor read.
 * 
 * @param sensor Pointer to DHT11 sensor structure
 * @return Temperature in degrees Celsius (0-50°C range)
 * 
 * @note Returns the cached value, does not perform a new sensor read
 * @note Call dht11_read() first to get fresh data
 */
int dht11_get_temperature(dht11_t *sensor);

/**
 * @brief Get the last read humidity value
 * 
 * Returns the humidity value from the most recent successful sensor read.
 * 
 * @param sensor Pointer to DHT11 sensor structure
 * @return Relative humidity as percentage (20-90% range)
 * 
 * @note Returns the cached value, does not perform a new sensor read
 * @note Call dht11_read() first to get fresh data
 */
int dht11_get_humidity(dht11_t *sensor);

/**
 * @brief Convert Celsius temperature to Fahrenheit
 * 
 * Utility function to convert temperature from Celsius to Fahrenheit scale.
 * Uses the formula: F = (C * 9/5) + 32
 * 
 * @param celsius Temperature in degrees Celsius
 * @return Temperature in degrees Fahrenheit
 * 
 * @note This is a pure conversion function, no sensor interaction
 * @example dht11_celsius_to_fahrenheit(25) returns 77.0°F
 */
float dht11_celsius_to_fahrenheit(int celsius);

#endif /*MAIN_DHT11_H_*/