
/**
 * @file LiquidCrystal_I2C.h
 * @brief I2C LCD Display Driver Header for ESP32 Weather Station
 * @details This header file defines the interface for controlling I2C LCD displays
 *          used in the ESP32 weather station project. It provides function prototypes,
 *          command definitions, and configuration constants for HD44780-compatible
 *          LCD displays connected via I2C interface using PCF8574 I/O expander.
 *          Supports 16x2 and 20x4 LCD configurations with backlight control.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 3, 2025 at 2:00 PM
 */

#ifndef LIQUID_CRYTAL_I2C_H_
#define LIQUID_CRYTAL_I2C_H_

#include <stdint.h>
#include "esp_err.h"

// SDA and SCL pin definitions
#define LCD_I2C_SDA_PIN 27  // Default SDA pin for I2C
#define LCD_I2C_SCL_PIN 26  // Default SCL pin for I2

// LCD Instructions
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE 0x04
#define LCD_DISPLAY_ON_OFF 0x08
#define LCD_CURSOR_ON_DISPLAY_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDRESS 0x40
#define LCD_SET_DDRAM_ADDRESS 0x80
#define LCD_READ_BUSY_FLAG_ADDRESS 0xF0
#define LCD_WRITE_DATA_RAM 0x80
#define LCD_READ_DATA_RAM 0xC0

// LCD Function Set Flags
#define LCD_8_BIT_MODE 0x10
#define LCD_4_BIT_MODE 0x00
#define LCD_2_LINE_MODE 0x08
#define LCD_1_LINE_MODE 0x00
#define LCD_5x10_DOTS_MODE 0x04
#define LCD_5x8_DOTS_MODE 0x00

// LCD Entry Mode Flags
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

// LCD Display On/Off Flags
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

// LCD Cursor/Display Shift Flags
#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE 0x00
#define LCD_MOVE_RIGHT 0x04
#define LCD_MOVE_LEFT 0x00

// LCD Busy Flag
#define LCD_BUSY_FLAG 0x80

// LCD I2C Address
// #define LCD_I2C_ADDRESS                 0x27

// LCD Backlight Control
#define LCD_BACKLIGHT_ON 0x08
#define LCD_BACKLIGHT_OFF 0x00

// LCD Commands
#define LCD_CMD_CLEAR (LCD_CLEAR_DISPLAY | LCD_BACKLIGHT_ON)
#define LCD_CMD_HOME (LCD_RETURN_HOME | LCD_BACKLIGHT_ON)
#define LCD_CMD_ENTRY_MODE (LCD_ENTRY_MODE | LCD_BACKLIGHT_ON)
#define LCD_CMD_DISPLAY_ON_OFF (LCD_DISPLAY_ON_OFF | LCD_BACKLIGHT_ON)
#define LCD_CMD_CURSOR_ON_DISPLAY_SHIFT (LCD_CURSOR_ON_DISPLAY_SHIFT | LCD_BACKLIGHT_ON)
#define LCD_CMD_FUNCTION_SET (LCD_FUNCTION_SET | LCD_BACKLIGHT_ON)
#define LCD_CMD_SET_CGRAM_ADDRESS (LCD_SET_CGRAM_ADDRESS | LCD_BACKLIGHT_ON)
#define LCD_CMD_SET_DDRAM_ADDRESS (LCD_SET_DDRAM_ADDRESS | LCD_BACKLIGHT_ON)
#define LCD_CMD_READ_BUSY_FLAG_ADDRESS (LCD_READ_BUSY_FLAG_ADDRESS | LCD_BACKLIGHT_ON)
#define LCD_CMD_WRITE_DATA_RAM (LCD_WRITE_DATA_RAM | LCD_BACKLIGHT_ON)
#define LCD_CMD_READ_DATA_RAM (LCD_READ_DATA_RAM | LCD_BACKLIGHT_ON)

// I2C PCF8574 Pin Mapping for LCD
#define LCD_RS_PIN    0x01    // Register Select pin
#define LCD_ENABLE_PIN 0x04   // Enable pin  
#define LCD_D4_PIN    0x10    // Data pin 4
#define LCD_D5_PIN    0x20    // Data pin 5
#define LCD_D6_PIN    0x40    // Data pin 6
#define LCD_D7_PIN    0x80    // Data pin 7

// Default I2C Configuration
#define LCD_I2C_ADDRESS         0x27    // Default I2C address for PCF8574
#define LCD_I2C_MASTER_PORT     I2C_NUM_0
#define LCD_I2C_MASTER_FREQ_HZ  100000  // 100kHz I2C frequency

// LCD Timing Constants
#define LCD_DELAY_ENABLE_PULSE  1       // Enable pulse width in microseconds
#define LCD_DELAY_COMMAND       2000    // Command execution delay in microseconds
#define LCD_DELAY_INIT          50000   // Initialization delay in microseconds

// Function prototypes
esp_err_t liquid_crystal_i2c_init(uint8_t addr, uint8_t cols, uint8_t rows);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char* str);
void lcd_print_char(char c);
void lcd_print_int(int num);
void lcd_print_float(float num, uint8_t decimals);
void begin(uint8_t cols, uint8_t rows, uint8_t charsize);
void home(void);
void no_display(void);
void display(void);
void no_blink(void);
void blink(void);
void no_cursor(void);
void cursor(void);
void scroll_display_left(void);
void scroll_display_right(void);
void print_left(void);
void print_right(void);
void left_to_right(void);
void right_to_left(void);
void shift_increment(void);
void shift_decrement(void);
void no_backlight(void);
void backlight(void);
void autoscroll(void);
void no_autoscroll(void);

#endif /* END LIQUID_CRYTAL_I2C_H_*/