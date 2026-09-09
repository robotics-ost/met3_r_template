/**
 * @file motor_driver.h
 * @brief Header file for the motor driver module.
 * This module provides functions to initialize and control the TB6612FNG motor driver.
 * It includes functions to set motor voltages, update bus voltage readings, and manage PWM channels.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 *
 * Example usage:
 * @code
 * // Initialize the motor driver with the desired configuration
 * motor_driver_config_t motor_config = {
 *     .bus_voltage_update_interval_ms = 10000,
 *     .min_bus_voltage = 5.0f
 * };
 * ESP_ERROR_CHECK(motor_driver_init(&motor_config));
 *
 * // Create a FreeRTOS task to periodically update the bus voltage
 * xTaskCreate(update_bus_voltage_task, "Update Bus Voltage", 2048, NULL, 1, NULL);
 *
 * // Set the motor driver state to enabled (should be done from the safety system)
 * motor_driver_set_state(MOTORS_ENABLED);
 *
 * // Set the voltage for motor A to 3.0V and motor B to -2.0V
 * motor_driver_set_voltage(MOTOR_A, 3.0f);
 * motor_driver_set_voltage(MOTOR_B, -2.0f);
 * @endcode
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>

/**
 * @brief Motor driver configuration structure
 */
typedef struct
{
    uint32_t bus_voltage_update_interval_ms; /**< Interval for updating bus voltage in milliseconds */
    float min_bus_voltage;                   /**< Minimum expected bus voltage */
    bool motor_A_free_spin;                  /**< Flag to indicate if motor A should be in free spin mode */
    bool motor_B_free_spin;                  /**< Flag to indicate if motor B should be in free spin mode */
} motor_driver_config_t;

/**
 * @brief Motor state
 */
typedef enum
{
    MOTORS_DISABLED = 0, /**< Motors are disabled (free spin mode) */
    MOTORS_SHORT_BRAKE,  /**< Motors are in short brake mode */
    MOTORS_ENABLED       /**< Motors are enabled */
} motor_driver_state_t;

/**
 * @brief Initialize the motor driver
 *
 * This function configures the GPIO pins and PWM channels for the TB6612FNG motor driver.
 * The initial drive states for both motors are set to "Stop" (both IN1 and IN2 low).
 * In addition, it initializes the INA219 I2C device to read and update the bus voltage.
 *
 * @param config Pointer to the motor driver configuration structure.
 * @return ESP_OK if initialization is successful, ESP_FAIL otherwise
 */
esp_err_t motor_driver_init(const motor_driver_config_t *config);

/**
 * @brief Sets the motor driver state
 * This function should only be called from the safety system.
 * @param state The desired motor driver state.
 */
void motor_driver_set_state(motor_driver_state_t state);

/**
 * @brief Set the voltage for both motors
 *
 * This function controls the direction and speed of both motors based on the input voltage.
 *
 * @param voltage Pointer to the array of desired voltages (positive for forward, negative for reverse)
 *                voltage[0]: Voltage for MOTOR_A
 *                voltage[1]: Voltage for MOTOR_B
 */
void motor_driver_set_voltage(const float voltage[2]);

/**
 * @brief Set the voltage for motor A
 *
 * This function can be used when only controlling motor A is desired.
 * In this case motor B should be set to free spin mode in the configuration.
 *
 * @param voltage Desired voltage for motor A (positive for forward, negative for reverse)
 */
void motor_driver_set_voltage_motor_A(float voltage);

/**
 * @brief Set the voltage for motor B
 *
 * This function can be used when only controlling motor B is desired.
 * In this case motor A should be set to free spin mode in the configuration.
 *
 * @param voltage Desired voltage for motor B (positive for forward, negative for reverse)
 */
void motor_driver_set_voltage_motor_B(float voltage);

/**
 * @brief Task to update the bus voltage periodically
 *
 * @param arg Pointer to task arguments
 */
void update_bus_voltage_task(void *arg);
