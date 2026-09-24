/**
 * @file pd_controller.h
 * @brief PD position controller header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>

/**
 * @brief PD position controller configuration structure
 */
typedef struct
{
    float dt;       /**< Control system period [s] */
    float kp;       /**< Proportional gain [1/s] */
    float kd;       /**< Derivative gain [1/s] */
    const float *M; /**< Pointer to the mass matrix [kg*m^2 or kg] */
    uint8_t dim;    /**< Dimension of the input signal (e.g., 1 for scalar, 2 for vector) */
} pd_controller_config_t;

/**
 * @brief Initializes the PD position controller with the given configuration.
 *
 * @param config Pointer to the PD position controller configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t pd_controller_init(const pd_controller_config_t *config);

/**
 * @brief Cleans up the PD position controller, freeing any allocated resources.
 */
void pd_controller_cleanup(void);

/**
 * @brief Runs the PD position controller to compute the control output.
 *
 * This function calculates the required torque or force Q to achieve the desired joint position.
 *
 * @param q Pointer to the current joint positions [rad or m].
 * @param q_d Pointer to the desired joint positions [rad or m].
 * @param Q Pointer to return the calculated joint torques/forces [Nm or N].
 */
void pd_controller_calc_Q(const float *q, const float *q_d, float *Q);
