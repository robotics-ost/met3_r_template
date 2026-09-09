/**
 * @file encoder_driver.h
 * @brief Header file for the encoder driver module.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 *
 * Example usage:
 * @code
 * // Initialize the encoder driver with the desired configuration
 * encoder_driver_config_t encoder_config = {
 *     .enc_A_pulses_per_revolution = 2048,
 *     .enc_B_pulses_per_revolution = 2048
 * };
 * ESP_ERROR_CHECK(encoder_driver_init(&encoder_config));
 *
 * // Get the shaft angles from both encoders (typically called in a loop or task)
 * float angles[2];
 * encoder_driver_get_shaft_angle(angles);
 * @endcode
 */

#pragma once

#include <esp_err.h>

/**
 * @brief Configuration structure for the encoder driver.
 */
typedef struct
{
    float enc_A_pulses_per_revolution; /**< Number of pulses per revolution for Encoder A */
    float enc_B_pulses_per_revolution; /**< Number of pulses per revolution for Encoder B */
} encoder_driver_config_t;

/**
 * @brief Initialize the encoder driver
 *
 * This function initializes both encoders and sets up the necessary configurations.
 *
 * @param config Pointer to the encoder driver configuration structure.
 * @return ESP_OK if initialization is successful, ESP_FAIL otherwise
 */
esp_err_t encoder_driver_init(const encoder_driver_config_t *config);

/**
 * @brief Get the angle from encoder A
 *
 * @note The returned angles are the input shaft angles in radians.
 *       The function does not account for the transmission ratio of the motors.
 *
 * @param angle Pointer to a float where the shaft angle will be stored in radians.
 */
void encoder_driver_get_angle_A(float *angle);

/**
 * @brief Get the angle from encoder B
 *
 * @note The returned angles are the input shaft angles in radians.
 *       The function does not account for the transmission ratio of the motors.
 *
 * @param angle Pointer to a float where the shaft angle will be stored in radians.
 */
void encoder_driver_get_angle_B(float *angle);

/**
 * @brief Get the angles from both encoders
 *
 * @note The returned angles are the input shaft angles in radians.
 *       The function does not account for the transmission ratio of the motors.
 *
 * @param angles Pointer to an array of two floats where the shaft angles will be stored in radians.
 *               angles[0] will hold the angle of ENCODER_A and angles[1] will hold the angle of ENCODER_B.
 */
void encoder_driver_get_angles(float angles[2]);
