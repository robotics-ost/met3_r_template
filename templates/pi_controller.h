/**
 * @file pi_controller.h
 * @brief PI velocity controller header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>

/**
 * @brief PI velocity controller configuration structure
 */
typedef struct
{
    float dt;             /**< Control system period [s] */
    float kp;             /**< Proportional gain [1/s] */
    float ki;             /**< Integral gain [1/s^2] */
    float integral_limit; /**< Limit for the integral term to prevent windup [rad or m] */
    const float *M;       /**< Pointer to the mass matrix [kg*m^2 or kg] */
    uint8_t dim;          /**< Dimension of the input signal (e.g., 1 for scalar, 2 for vector) */
} pi_controller_config_t;

/**
 * @brief Initializes the PI velocity controller with the given configuration.
 *
 * @param config Pointer to the PI velocity controller configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t pi_controller_init(const pi_controller_config_t *config);

/**
 * @brief Cleans up the PI velocity controller, freeing any allocated resources.
 */
void pi_controller_cleanup(void);

/**
 * @brief Disables the integrator in the PI controller.
 *
 * This function clears the integrator enabled bit in the event group, effectively disabling the integration of the error term.
 * This is useful in scenarios where you want to prevent the integrator from accumulating error, such as during system shutdown or emergency conditions.
 */
void pi_controller_disable_integrator(void);

/**
 * @brief Enables the integrator in the PI controller.
 *
 * This function sets the integrator enabled bit in the event group, effectively enabling the integration of the error term.
 */
void pi_controller_enable_integrator(void);

/**
 * @brief Resets the integrator in the PI controller.
 *
 * This function clears the integrator state, effectively resetting the accumulated error to zero.
 */
void pi_controller_reset_integrator(void);

/**
 * @brief Runs the PI velocity controller to compute the control output.
 *
 * This function calculates the required torque or force Q to achieve the desired joint velocity.
 *
 * The integral terms are limited to prevent windup.
 *
 * @param qd Pointer to the current joint velocities [rad/s or m/s].
 * @param qd_d Pointer to the desired joint velocities [rad/s or m/s].
 * @param Q Pointer to return the calculated joint torques/forces [Nm or N].
 */
void pi_controller_calc_Q(const float *qd, const float *qd_d, float *Q);
