/**
 * @file udp_interface.h
 * @brief UDP interface for handling network communication.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 *
 * Example usage:
 * @code
 * // Initialize UDP interface configuration
 * udp_interface_config_t udp_config = {
 *     .wifi_ssid = "MyWiFi",
 *     .wifi_pass = "MyPassword",
 *     .hostname = "robot_1",
 *     .udp_port = 3333,
 *     .task_period_ms = 20,
 *     .telemetry_queue_len = 64
 * };
 * udp_interface_init(&udp_config);
 *
 * // Start the UDP interface task
 * xTaskCreate(udp_interface_task, "udp_interface_task", 4096, NULL, 3, NULL);
 *
 * // Add telemetry samples
 * udp_interface_add_safety_sample(safety_level);
 * udp_interface_add_control_sample(x, y, phi, v, omega);
 * @endcode
 */

#pragma once

#include <stdint.h>
#include <esp_err.h>
#include "safety_system.h"

/**
 * @brief Configuration structure for the UDP interface.
 */
typedef struct
{
    uint8_t wifi_ssid[32];   /**< WiFi SSID */
    uint8_t wifi_pass[64];   /**< WiFi password */
    char *hostname;          /**< Desired hostname for the ESP32 on the network */
    uint16_t udp_port;       /**< UDP port for communication */
    uint32_t task_period_ms; /**< Network task period in milliseconds */
    int telemetry_queue_len; /**< Length of the telemetry queue */
} udp_interface_config_t;

/**
 * @brief Initialize the UDP interface.
 *
 * This function sets up the WiFi interface, connects to the specified SSID,
 * and waits until the connection is established.
 * When the connection is successful, the WIFI_CONNECTED_BIT is set in the
 * wifi_event_group.
 * Upon failure, the function will log an error and may retry the connection.
 * In addition it initializes the telemetry queues for safety and control samples.
 *
 * @return ESP_OK if initialization is successful, ESP_FAIL otherwise
 */
esp_err_t udp_interface_init(const udp_interface_config_t *config);

/**
 * @brief Add a telemetry safety sample to the queue.
 *
 * This function is called by the safety system to add a new telemetry safety sample.
 * The sample will be sent to the host over UDP in the next batch.
 *
 * @param safety_level The current hierarchical safety level
 */
void udp_interface_add_safety_sample(safety_level_t safety_level);

/**
 * @brief Add a telemetry control sample to the queue.
 *
 * This function is called by the control system to add a new telemetry control sample.
 * The sample will be sent to the host over UDP in the next batch.
 *
 * @param enc_A_pos Encoder A position (rad)
 * @param enc_B_pos Encoder B position (rad)
 */
void udp_interface_add_control_sample(float enc_A_pos, float enc_B_pos);

/**
 * @brief UDP interface task
 *
 * This task handles UDP communication and message processing.
 *
 * @param arg Unused parameter for task creation.
 */
void udp_interface_task(void *arg);
