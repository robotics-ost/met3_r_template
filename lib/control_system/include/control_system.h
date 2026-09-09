/**
 * @file control_system.h
 * @brief Control system header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 *
 * Example usage:
 * @code
 * // Initialize the control system with the desired configuration
 * control_system_config_t config = {
 *     .task_period_ms = 10,
 *     .motor_model_config = { ... },
 *     .kinematics_config = { ... }
 *     ...
 * };
 * control_system_init(&config);
 *
 * // Start the control system task
 * xTaskCreate(control_system_task, "control_system_task", 4096, NULL, 5, NULL);
 * @endcode
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>
#include "encoder_driver.h"
#include "motor_driver.h"
/* Add other necessary includes here */

/**
 * @brief Control system configuration structure
 */
typedef struct
{
    uint32_t task_period_ms;                   /**< Control system task period in milliseconds */
    encoder_driver_config_t encoder_config;    /**< Configuration for the encoder driver */
    motor_driver_config_t motor_driver_config; /**< Configuration for the motor driver */
    /* Add other configuration structures here */
} control_system_config_t;

/**
 * @brief Initializes the control system.
 *
 * This function sets up the control system, including creating the necessary event group for synchronization.
 * It should be called before starting the control system task.
 *
 * @param config Pointer to the control system configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t control_system_init(const control_system_config_t *config);

/**
 * @brief Control system task.
 *
 * This task is responsible for executing the control system logic, including reading sensors,
 * computing control outputs, and sending commands to actuators.
 * It waits for the control system to be initialized before starting its main loop.
 * The task runs periodically based on the defined task period in the configuration.
 *
 * @param arg Argument passed to the task (not used).
 */
void control_system_task(void *arg);
