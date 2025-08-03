
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

// Global variables for LCD state management
static const char *TAG = "LCD_I2C";                          // Logging tag for ESP-IDF logging system
static uint8_t lcd_addr = 0x27;                              // Default I2C address of LCD (can be 0x3F on some modules)
static uint8_t lcd_cols = 16;                                // Number of character columns on display
static uint8_t lcd_rows = 2;                                 // Number of character rows on display
static uint8_t lcd_backlight = LCD_BACKLIGHT_ON;             // Current backlight state (on/off)
static bool i2c_initialized = false;                         // Flag to prevent double initialization of I2C

// Static function prototypes for internal LCD operations
static esp_err_t i2c_master_init(void);                      // Initialize ESP32 I2C master interface
static esp_err_t lcd_write_byte(uint8_t data);               // Send single byte over I2C to PCF8574
static void lcd_write_nibble(uint8_t nibble);                // Send 4-bit data nibble to LCD
static void lcd_send_command(uint8_t cmd);                   // Send command to LCD (RS=0)
static void lcd_send_data(uint8_t data);                     // Send character data to LCD (RS=1)
static void lcd_pulse_enable(uint8_t data);                  // Generate enable pulse for LCD timing

// Initialize I2C master interface for LCD communication
static esp_err_t i2c_master_init(void)
{
    if (i2c_initialized) 
    {
        return ESP_OK;  // Skip initialization if already done to prevent conflicts
    }

    // Configure I2C master parameters for LCD communication
    i2c_config_t conf = 
    {
        .mode = I2C_MODE_MASTER,                              // Set ESP32 as I2C master device
        .sda_io_num = LCD_I2C_SDA_PIN,                        // Serial Data line pin (GPIO 21 default)
        .scl_io_num = LCD_I2C_SCL_PIN,                        // Serial Clock line pin (GPIO 22 default)
        .sda_pullup_en = GPIO_PULLUP_ENABLE,                  // Enable internal pull-up on SDA (4.7kΩ typical)
        .scl_pullup_en = GPIO_PULLUP_ENABLE,                  // Enable internal pull-up on SCL (4.7kΩ typical)
        .master.clk_speed = LCD_I2C_MASTER_FREQ_HZ,           // Set I2C clock frequency (100kHz standard)
    };

    esp_err_t err = i2c_param_config(LCD_I2C_MASTER_PORT, &conf);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Install I2C driver with configuration (no RX/TX buffers needed for master)
    err = i2c_driver_install(LCD_I2C_MASTER_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_initialized = true;                                   // Mark I2C as successfully initialized
    ESP_LOGI(TAG, "I2C master initialized successfully");
    return ESP_OK;
}

// Write a single byte to the I2C LCD device (PCF8574 I/O expander)
static esp_err_t lcd_write_byte(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();             // Create I2C command sequence container
    i2c_master_start(cmd);                                    // Generate I2C start condition
    i2c_master_write_byte(cmd, (lcd_addr << 1) | I2C_MASTER_WRITE, true);  // Send device address + write bit with ACK check
    i2c_master_write_byte(cmd, data, true);                   // Send data byte to PCF8574 with ACK check
    i2c_master_stop(cmd);                                     // Generate I2C stop condition
    
    esp_err_t ret = i2c_master_cmd_begin(LCD_I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(100));  // Execute command with 100ms timeout
    i2c_cmd_link_delete(cmd);                                 // Free command sequence memory
    
    return ret;                                               // Return I2C transaction result
}

// Send 4-bit nibble to LCD via I2C (used in 4-bit mode communication)
static void lcd_write_nibble(uint8_t nibble)
{
    uint8_t data = nibble | lcd_backlight;                    // Combine nibble with current backlight state
    lcd_write_byte(data);                                     // Send data to PCF8574 (data + backlight control)
    lcd_pulse_enable(data);                                   // Generate enable pulse to latch data into LCD
}

// Generate enable pulse for LCD data latching (critical timing requirement)
static void lcd_pulse_enable(uint8_t data)
{
    lcd_write_byte(data | LCD_ENABLE_PIN);                    // Set enable pin high (start of pulse)
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE);                 // Hold enable high for required duration (1μs min)
    lcd_write_byte(data & ~LCD_ENABLE_PIN);                   // Set enable pin low (end of pulse)
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE);                 // Hold enable low for required duration (1μs min)
}

// Send command to LCD controller (RS=0 for command mode)
static void lcd_send_command(uint8_t cmd)
{
    uint8_t upper_nibble = cmd & 0xF0;                        // Extract upper 4 bits (mask with 11110000)
    uint8_t lower_nibble = (cmd << 4) & 0xF0;                 // Shift lower 4 bits to upper position
    
    // Send upper nibble first (HD44780 protocol requirement)
    lcd_write_nibble(upper_nibble);                           // RS=0 (low) indicates command mode
    // Send lower nibble second to complete 8-bit command
    lcd_write_nibble(lower_nibble);                           // RS=0 (low) indicates command mode
    
    esp_rom_delay_us(LCD_DELAY_COMMAND);                      // Wait for command execution (50μs typical)
}

// Send character data to LCD controller (RS=1 for data mode)
static void lcd_send_data(uint8_t data)
{
    uint8_t upper_nibble = data & 0xF0;                       // Extract upper 4 bits of character data
    uint8_t lower_nibble = (data << 4) & 0xF0;                // Shift lower 4 bits to upper position
    
    // Send upper nibble with RS high to indicate data mode
    lcd_write_nibble(upper_nibble | LCD_RS_PIN);              // RS=1 (high) indicates data/character mode
    // Send lower nibble with RS high to complete character transfer
    lcd_write_nibble(lower_nibble | LCD_RS_PIN);              // RS=1 (high) indicates data/character mode
    
    esp_rom_delay_us(LCD_DELAY_COMMAND);                      // Wait for data processing (50μs typical)
}

// Initialize LCD display with I2C interface (main initialization function)
esp_err_t liquid_crystal_i2c_init(uint8_t addr, uint8_t cols, uint8_t rows)
{
    esp_err_t ret;
    
    // Store LCD configuration parameters for later use
    lcd_addr = addr;                                          // Store I2C address (0x27 or 0x3F typically)
    lcd_cols = cols;                                          // Store number of columns (16 or 20 typically)
    lcd_rows = rows;                                          // Store number of rows (2 or 4 typically)
    
    // Initialize ESP32 I2C master interface
    ret = i2c_master_init();
    if (ret != ESP_OK) 
    {
        return ret;                                           // Return error if I2C initialization fails
    }
    
    ESP_LOGI(TAG, "Initializing LCD at address 0x%02X (%dx%d)", addr, cols, rows);
    
    // Critical LCD initialization sequence - timing is essential for reliability
    vTaskDelay(pdMS_TO_TICKS(50));                            // Wait 50ms for LCD power stabilization
    
    // HD44780 power-on initialization sequence (must be exact for 4-bit mode)
    lcd_write_nibble(0x30);                                   // Function set: 8-bit mode (first attempt)
    vTaskDelay(pdMS_TO_TICKS(5));                             // Wait 5ms (longer delay for first command)
    lcd_write_nibble(0x30);                                   // Function set: 8-bit mode (second attempt)
    vTaskDelay(pdMS_TO_TICKS(1));                             // Wait 1ms (shorter delay for subsequent commands)
    lcd_write_nibble(0x30);                                   // Function set: 8-bit mode (third attempt)
    vTaskDelay(pdMS_TO_TICKS(1));                             // Wait 1ms before switching modes
    lcd_write_nibble(0x20);                                   // Function set: 4-bit mode (critical transition)
    vTaskDelay(pdMS_TO_TICKS(1));                             // Wait for mode switch completion
    
    // Configure LCD operating parameters using 4-bit commands
    lcd_send_command(LCD_FUNCTION_SET | LCD_4_BIT_MODE | LCD_2_LINE_MODE | LCD_5x8_DOTS_MODE);  // Set 4-bit, 2-line, 5x8 font
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF);   // Turn display off during configuration
    lcd_send_command(LCD_CLEAR_DISPLAY);                      // Clear display memory (all spaces)
    vTaskDelay(pdMS_TO_TICKS(2));                             // Clear command requires longer execution time
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);  // Set cursor increment, no auto-shift
    
    // Enable display with desired cursor settings
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);  // Display on, cursor off, blink off
    
    // Turn on backlight for visibility
    backlight();                                              // Enable LCD backlight
    
    ESP_LOGI(TAG, "LCD initialization completed successfully");
    return ESP_OK;
}

// Clear entire LCD display and return cursor to home position (0,0)
void lcd_clear(void)
{
    lcd_send_command(LCD_CLEAR_DISPLAY);                      // Send clear command to HD44780 controller
    vTaskDelay(pdMS_TO_TICKS(2));                             // Clear command needs extra execution time (1.52ms typical)
}

// Return cursor to home position (0,0) without clearing display content
void lcd_home(void)
{
    lcd_send_command(LCD_RETURN_HOME);                        // Send home command to HD44780 controller
    vTaskDelay(pdMS_TO_TICKS(2));                             // Home command needs extra execution time (1.52ms typical)
}

// Set cursor position for next character output (zero-based coordinates)
void lcd_set_cursor(uint8_t col, uint8_t row)
{
    // DDRAM address offsets for different LCD configurations
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};        // Row 0, 1, 2, 3 start addresses in DDRAM
    
    // Boundary checking to prevent invalid memory access
    if (row >= lcd_rows) {
        row = lcd_rows - 1;                                   // Clamp row to maximum valid value
    }
    if (col >= lcd_cols) {
        col = lcd_cols - 1;                                   // Clamp column to maximum valid value
    }
    
    uint8_t address = col + row_offsets[row];                 // Calculate DDRAM address (row offset + column)
    lcd_send_command(LCD_SET_DDRAM_ADDRESS | address);        // Send set DDRAM address command (0x80 | address)
}

// Print null-terminated string to LCD at current cursor position
void lcd_print(const char* str)
{
    if (str == NULL) return;                                  // Safety check for null pointer
    
    while (*str) {                                            // Loop through each character until null terminator
        lcd_send_data(*str++);                                // Send character data and increment pointer
    }
}

// Print single character to LCD at current cursor position
void lcd_print_char(char c)
{
    lcd_send_data(c);                                         // Send character directly as data byte
}

// Print integer value with automatic string conversion
void lcd_print_int(int num)
{
    char buffer[12];                                          // Buffer large enough for 32-bit signed integer
    snprintf(buffer, sizeof(buffer), "%d", num);             // Convert integer to string representation
    lcd_print(buffer);                                        // Print the converted string
}

// Print floating-point number with specified decimal places
void lcd_print_float(float num, uint8_t decimals)
{
    char buffer[16];                                          // Buffer for floating-point string representation
    char format[8];                                           // Buffer for format string creation
    
    // Create format string dynamically (e.g., "%.2f" for 2 decimal places)
    snprintf(format, sizeof(format), "%%.%df", decimals);    // Build format string with specified decimals
    snprintf(buffer, sizeof(buffer), format, num);           // Convert float using custom format
    lcd_print(buffer);                                        // Print the formatted string
}

// Legacy compatibility function for Arduino-style LCD initialization
void begin(uint8_t cols, uint8_t rows, uint8_t charsize)
{
    lcd_cols = cols;                                          // Store column count for boundary checking
    lcd_rows = rows;                                          // Store row count for boundary checking
    // charsize parameter is ignored - HD44780 uses 5x8 dots standard character format
}

// Legacy compatibility function - return cursor to home position
void home(void)
{
    lcd_home();                                               // Call the main home function
}

// Turn LCD display output off (preserves display memory content)
void no_display(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF);  // Display off, cursor off, blink off
}

// Turn LCD display output on (restores display memory content)
void display(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);   // Display on, cursor off, blink off
}

// Turn cursor blinking effect off (cursor remains solid if enabled)
void no_blink(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);   // Display on, cursor off, blink off
}

// Turn cursor blinking effect on (cursor blinks at current position)
void blink(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON);     // Display on, cursor on, blink on
}

// Turn cursor visibility off (no cursor indicator shown)
void no_cursor(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);   // Display on, cursor off, blink off
}

// Turn cursor visibility on (shows underscore at current position)
void cursor(void)
{
    lcd_send_command(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_OFF);    // Display on, cursor on, blink off
}

// Scroll entire display content one position to the left
void scroll_display_left(void)
{
    lcd_send_command(LCD_CURSOR_ON_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT);         // Shift display content left
}

// Scroll entire display content one position to the right
void scroll_display_right(void)
{
    lcd_send_command(LCD_CURSOR_ON_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT);        // Shift display content right
}

// Set text entry direction from left to right (normal reading order)
void left_to_right(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);           // Cursor increments right, no auto-shift
}

// Set text entry direction from right to left (reverse reading order)
void right_to_left(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_RIGHT | LCD_ENTRY_SHIFT_DECREMENT);          // Cursor increments left, no auto-shift
}

// Enable automatic scrolling when text reaches display edge
void autoscroll(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_INCREMENT);           // Cursor moves right, display auto-shifts
}

// Disable automatic scrolling (text wraps to next line or clips)
void no_autoscroll(void)
{
    lcd_send_command(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);           // Cursor moves right, no auto-shift
}

// Turn LCD backlight off for power saving or dimming
void no_backlight(void)
{
    lcd_backlight = LCD_BACKLIGHT_OFF;                        // Update global backlight state
    lcd_write_byte(0x00);                                     // Send all zeros to turn off backlight LED
}

// Turn LCD backlight on for normal visibility
void backlight(void)
{
    lcd_backlight = LCD_BACKLIGHT_ON;                         // Update global backlight state
    lcd_write_byte(lcd_backlight);                            // Send backlight control bit to PCF8574
}

// Legacy compatibility functions for alternative naming conventions
void print_left(void) { left_to_right(); }                   // Alias for left_to_right() function
void print_right(void) { right_to_left(); }                  // Alias for right_to_left() function
void shift_increment(void) { autoscroll(); }                 // Alias for autoscroll() function
void shift_decrement(void) { no_autoscroll(); }              // Alias for no_autoscroll() function

