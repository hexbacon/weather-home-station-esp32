/**
 * @file tasks_common.h
 * @brief Common Task Configuration Header for ESP32 Weather Station
 * @details This header file defines common FreeRTOS task configuration
 *          parameters for the ESP32 weather station project. It centralizes
 *          task priorities, stack sizes, and CPU core assignments for all
 *          application tasks including WiFi management, HTTP server, sensor
 *          reading, and display updates to ensure optimal system performance
 *          and resource allocation.
 *
 * @author christophermena
 * @date July 30, 2025
 * @version 1.0
 * @note Last Updated: August 1, 2025 at 10:30 AM
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/**
 * @file tasks_common.h
 * @brief Common task configuration parameters for FreeRTOS tasks
 * 
 * This file defines the stack sizes, priorities, and core assignments
 * for all application tasks. Centralizing these configurations makes
 * it easier to tune system performance and manage task scheduling.
 * 
 * Priority Guidelines:
 * - Higher numbers = Higher priority (0 = lowest, configMAX_PRIORITIES-1 = highest)
 * - Critical tasks: 5-7, Normal tasks: 2-4, Background tasks: 0-1
 * 
 * Core Assignment:
 * - Core 0: Protocol tasks (WiFi, networking)
 * - Core 1: Application tasks (sensors, processing)
 */

// WiFi Application Task Configuration
#define WIFI_APP_TASK_STACK_SIZE            4096        ///< Stack size in bytes for WiFi application task
#define WIFI_APP_TASK_PRIORITY              5           ///< Task priority (high - handles critical WiFi events)
#define WIFI_APP_TASK_CORE_ID               0           ///< CPU core assignment (Core 0 - protocol stack)

// HTTP Server Task Configuration  
#define HTTP_SERVER_TASK_STACK_SIZE         8192        ///< Stack size in bytes for HTTP server task (larger for web content)
#define HTTP_SERVER_TASK_PRIORITY           4           ///< Task priority (high-normal - web server responsiveness)
#define HTTP_SERVER_TASK_CORE_ID            0           ///< CPU core assignment (Core 0 - networking)

// HTTP Server Monitor Task Configuration
#define HTTP_SERVER_MONITOR_STACK_SIZE      4096        ///< Stack size in bytes for HTTP monitor task
#define HTTP_SERVER_MONITOR_PRIORITY        3           ///< Task priority (normal - status monitoring)
#define HTTP_SERVER_MONITOR_CORE_ID         0           ///< CPU core assignment (Core 0 - networking related)

// DHT11 Sensor Task Configuration
#define DHT_SENSOR_TASK_STACK_SIZE          4096        ///< Stack size in bytes for DHT11 sensor task
#define DHT_SENSOR_TASK_PRIORITY            2           ///< Task priority (low-normal - periodic sensor reading)
#define DHT_SENSOR_TASK_CORE_ID             0           ///< CPU core assignment (Core 0 - application task)

#endif /* MAIN_TASKS_COMMON_H_ */