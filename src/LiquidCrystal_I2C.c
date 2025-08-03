
/**
 * @file LiquidCrystal_I2C.c
 * @brief I2C LCD Display Driver Implementation for ESP32 Weather Station
 * @details This file implements the I2C LCD display driver for the ESP32 weather
 *          station project. It provides complete functionality for controlling
 *          HD44780-compatible LCD displays via I2C interface using PCF8574 I/O
 *          expander. The implementation includes initialization, text display,
 *          cursor control, backlight management, and specialized functions for
 *          weather data visualization with proper timing and error handling.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 3, 2025 at 2:00 PM
 */

#include "LiquidCrystal_I2C.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"
#include <stdio.h>
#include <string.h>

// Global variables
static const char *TAG = "LCD_I2C";
static uint8_t lcd_addr = 0x27;          // I2C address of LCD
static uint8_t lcd_cols = 16;            // Number of columns
static uint8_t lcd_rows = 2;             // Number of rows
static uint8_t lcd_backlight = LCD_BACKLIGHT_ON;  // Backlight state
static bool i2c_initialized = false;    // I2C initialization flag

// Static function prototypes
static esp_err_t i2c_master_init(void);
static esp_err_t lcd_write_byte(uint8_t data);
static void lcd_write_nibble(uint8_t nibble);
static void lcd_send_command(uint8_t cmd);
static void lcd_send_data(uint8_t data);
static void lcd_pulse_enable(uint8_t data);

// Initialize I2C master
static esp_err_t i2c_master_init(void)
{
    if (i2c_initialized) 
    {
        return ESP_OK;  // Already initialized
    }

    i2c_config_t conf = 
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = LCD_I2C_SDA_PIN,      // Default SDA pin
        .scl_io_num = LCD_I2C_SCL_PIN,      // Default SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LCD_I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(LCD_I2C_MASTER_PORT, &conf);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(LCD_I2C_MASTER_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_initialized = true;
    ESP_LOGI(TAG, "I2C master initialized successfully");
    return ESP_OK;
}

// Write a byte to I2C device
static esp_err_t lcd_write_byte(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(LCD_I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

// Send nibble (4 bits) to LCD
static void lcd_write_nibble(uint8_t nibble)
{
    uint8_t data = nibble | lcd_backlight;
    lcd_write_byte(data);
    lcd_pulse_enable(data);
}

// Pulse the enable pin
static void lcd_pulse_enable(uint8_t data)
{
    lcd_write_byte(data | LCD_ENABLE_PIN);  // Set enable high
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE);
    lcd_write_byte(data & ~LCD_ENABLE_PIN); // Set enable low
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE);
}

// Send command to LCD
static void lcd_send_command(uint8_t cmd)
{
    uint8_t upper_nibble = cmd & 0xF0;
    uint8_t lower_nibble = (cmd << 4) & 0xF0;
    
    // Send upper nibble
    lcd_write_nibble(upper_nibble);
    // Send lower nibble  
    lcd_write_nibble(lower_nibble);
    
    esp_rom_delay_us(LCD_DELAY_COMMAND);
}

// Send data to LCD
static void lcd_send_data(uint8_t data)
{
    uint8_t upper_nibble = data & 0xF0;
    uint8_t lower_nibble = (data << 4) & 0xF0;
    
    // Send upper nibble with RS high
    lcd_write_nibble(upper_nibble | LCD_RS_PIN);
    // Send lower nibble with RS high
    lcd_write_nibble(lower_nibble | LCD_RS_PIN);
    
    esp_rom_delay_us(LCD_DELAY_COMMAND);
}

// Initialize LCD with I2C
esp_err_t LiquidCrystal_I2C_Init(uint8_t addr, uint8_t cols, uint8_t rows)
{
    esp_err_t ret;
    
    // Store LCD parameters
    lcd_addr = addr;
    lcd_cols = cols;
    lcd_rows = rows;
    
    // Initialize I2C master
    ret = i2c_master_init();
    if (ret != ESP_OK) 
    {
        return ret;
    }
    
    ESP_LOGI(TAG, "Initializing LCD at address 0x%02X (%dx%d)", addr, cols, rows);
    
    // LCD initialization sequence
    vTaskDelay(pdMS_TO_TICKS(50));  // Wait for LCD power-up
    
    // Set LCD to 4-bit mode - this sequence is critical
    lcd_write_nibble(0x30);         // Function set: 8-bit mode
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write_nibble(0x30);         // Function set: 8-bit mode
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_write_nibble(0x30);         // Function set: 8-bit mode
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_write_nibble(0x20);         // Function set: 4-bit mode
    vTaskDelay(pdMS_TO_TICKS(1));
    
    // Configure LCD settings
    lcd_send_command(LCD_FUNCTION_SET | LCD_4_BIT_MODE | LCD_2_LINE_MODE | LCD_5x8_DOTS_MODE);
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF);
    lcd_send_command(LCD_CLEAR_DISPLAY);
    vTaskDelay(pdMS_TO_TICKS(2));   // Clear command takes longer
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);
    
    // Turn on display with no cursor or blink
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    
    // Turn on backlight
    backlight();
    
    ESP_LOGI(TAG, "LCD initialization completed successfully");
    return ESP_OK;
}

// Clear the display
void lcd_clear(void)
{
    lcd_send_command(LCD_CLEAR_DISPLAY);
    vTaskDelay(pdMS_TO_TICKS(2));  // Clear command needs extra time
}

// Return cursor to home position (0,0)
void lcd_home(void)
{
    lcd_send_command(LCD_RETURN_HOME);
    vTaskDelay(pdMS_TO_TICKS(2));  // Home command needs extra time
}

// Set cursor position
void lcd_setCursor(uint8_t col, uint8_t row)
{
    // Define row offsets for different LCD sizes
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    
    if (row >= lcd_rows) {
        row = lcd_rows - 1;  // Clamp to valid row
    }
    if (col >= lcd_cols) {
        col = lcd_cols - 1;  // Clamp to valid column
    }
    
    uint8_t address = col + row_offsets[row];
    lcd_send_command(LCD_SET_DDRAM_ADDRESS | address);
}

// Print a string to LCD
void lcd_print(const char* str)
{
    if (str == NULL) return;
    
    while (*str) {
        lcd_send_data(*str++);
    }
}

// Print a single character
void lcd_printChar(char c)
{
    lcd_send_data(c);
}

// Print an integer
void lcd_printInt(int num)
{
    char buffer[12];  // Enough for 32-bit int
    snprintf(buffer, sizeof(buffer), "%d", num);
    lcd_print(buffer);
}

// Print a float with specified decimal places
void lcd_printFloat(float num, uint8_t decimals)
{
    char buffer[16];
    char format[8];
    
    // Create format string like "%.2f"
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buffer, sizeof(buffer), format, num);
    lcd_print(buffer);
}

// Legacy function for compatibility
void begin(uint8_t cols, uint8_t rows, uint8_t charsize)
{
    lcd_cols = cols;
    lcd_rows = rows;
    // charsize parameter is ignored - HD44780 uses 5x8 dots
}

// Return to home position
void home(void)
{
    lcd_home();
}

// Turn display off
void no_display(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF);
}

// Turn display on
void display(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
}

// Turn cursor blinking off
void no_blink(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
}

// Turn cursor blinking on
void blink(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON);
}

// Turn cursor off
void no_cursor(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
}

// Turn cursor on
void cursor(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_OFF);
}

// Scroll display content left
void scroll_display_left(void)
{
    lcd_send_command(LCD_CURSOR_ON_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT);
}

// Scroll display content right
void scroll_display_right(void)
{
    lcd_send_command(LCD_CURSOR_ON_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT);
}

// Set text flow direction left-to-right
void left_to_right(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);
}

// Set text flow direction right-to-left
void right_to_left(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_RIGHT | LCD_ENTRY_SHIFT_DECREMENT);
}

// Turn on automatic scrolling
void autoscroll(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_INCREMENT);
}

// Turn off automatic scrolling
void no_autoscroll(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);
}

// Turn off backlight
void no_backlight(void)
{
    lcd_backlight = LCD_BACKLIGHT_OFF;
    lcd_write_byte(0x00);  // Send all zeros to turn off backlight
}

// Turn on backlight
void backlight(void)
{
    lcd_backlight = LCD_BACKLIGHT_ON;
    lcd_write_byte(lcd_backlight);  // Send backlight bit
}

// These functions are for compatibility but not typically used
void print_left(void) { left_to_right(); }
void print_right(void) { right_to_left(); }
void shift_increment(void) { autoscroll(); }
void shift_decrement(void) { no_autoscroll(); }

