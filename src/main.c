/**
 * @file main.c
 * @brief Main application entry point.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include <nvs_flash.h>
#include <driver/i2c_master.h>
#include <soc/clk_tree_defs.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "udp_interface.h"
#include "safety_system.h"
#include "control_system.h"
#include "sequencer.h"

#include "config.h"
#include "wifi_secrets.h"

static const char *TAG = "Main Application"; /**< Tag for logging purposes */

/**
 * @brief Application entry point.
 *
 * Initializes system resources and starts system tasks.
 */
void app_main()
{
    /* Initialization */
    ESP_LOGI(TAG, "Starting application initialization...");

    ESP_ERROR_CHECK(nvs_flash_init());

    const safety_system_config_t safety_config = {
        .task_period_ms = SAFETY_SYSTEM_TASK_PERIOD_MS,
        .event_queue_length = SAFETY_SYSTEM_EVENT_QUEUE_LENGTH,
    };
    ESP_ERROR_CHECK(safety_system_init(&safety_config));

    const i2c_master_bus_config_t conf = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&conf, &bus_handle));

    const udp_interface_config_t udp_config = {
        .wifi_ssid = UPD_INTERFACE_WIFI_SSID,
        .wifi_pass = UPD_INTERFACE_WIFI_PASS,
        .hostname = UPD_INTERFACE_HOSTNAME,
        .udp_port = UPD_INTERFACE_UDP_PORT,
        .task_period_ms = UPD_INTERFACE_TASK_PERIOD_MS,
        .telemetry_queue_len = UPD_INTERFACE_TELEMETRY_QUEUE_LEN,
    };
    ESP_ERROR_CHECK(udp_interface_init(&udp_config));

    const control_system_config_t control_config = {
        .task_period_ms = CONTROL_SYSTEM_TASK_PERIOD_MS,
        .encoder_config = {
            .enc_A_pulses_per_revolution = ENCODER_DRIVER_ENC_A_PPR,
            .enc_B_pulses_per_revolution = ENCODER_DRIVER_ENC_B_PPR,
        },
        .motor_driver_config = {
            .bus_voltage_update_interval_ms = MOTOR_DRIVER_BUS_VOLTAGE_UPDATE_PERIOD_MS,
            .min_bus_voltage = MOTOR_DRIVER_MIN_BUS_VOLTAGE,
            .motor_A_free_spin = MOTOR_DRIVER_MOTOR_A_FREE_SPIN,
            .motor_B_free_spin = MOTOR_DRIVER_MOTOR_B_FREE_SPIN,
        }
    };
    ESP_ERROR_CHECK(control_system_init(&control_config));

    sequencer_config_t sequencer_config = {
        .task_period_ms = SEQUENCER_TASK_PERIOD_MS,
    };
    ESP_ERROR_CHECK(sequencer_init(&sequencer_config));

    ESP_LOGI(TAG, "Initialization complete. Starting tasks...");

    /* Core 1: high-priority real time tasks (safety, control)
     * Core 0: low-priority tasks (network, sequencer) */
    xTaskCreatePinnedToCore(safety_system_task, "safety_system", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(control_system_task, "control_system", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(udp_interface_task, "udp_interface_task", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(update_bus_voltage_task, "update_bus_voltage_task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(sequencer_task, "sequencer_task", 4096, NULL, 3, NULL, 0);

    safety_system_register_event(SE_SYSTEM_INITIALIZED);
    ESP_LOGI(TAG, "Application started.");
}
