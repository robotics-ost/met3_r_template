/**
 * @file motor_model.h
 * @brief Motor model header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>

/**
 * @brief Motor parameters structure
 */
typedef struct
{
    float transmission_ratio;      /**< Motor transmission ratio [- or 1/m] */
    float armature_resistance;     /**< Motor armature resistance [Ohm] */
    float motor_constant;          /**< Motor constant [Nm/A] */
    float max_torque_intermittent; /**< Maximum intermittent input shaft torque [Nm] */
    float max_velocity;            /**< Maximum input shaft velocity [rad/s] */
    bool invert_direction;         /**< Flag to indicate if the motor direction should be inverted */
} motor_model_params_t;

/**
 * @brief Motor configuration structure
 */
typedef struct
{
    float dt;                           /**< Control system period [s] */
    const motor_model_params_t *params; /**< Array of per-motor parameters */
    uint8_t nr_of_motors;               /**< Number of motors */
} motor_model_config_t;

/**
 * @brief Initializes the motor(s) with the given configuration.
 *
 * @param config Pointer to the motor configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t motor_model_init(const motor_model_config_t *config);

/**
 * @brief Cleans up the motor model, freeing any allocated resources.
 */
void motor_model_cleanup(void);

/**
 * @brief Calculates the joint position q from the input shaft angle.
 *
 * @note Depending on the number of motors set in the configuration,
 * this function accepts either a scalar or a vector.
 *
 * @param shaft_angles Pointer to current input shaft angle [rad].
 * @param q Pointer to return the calculated joint position [rad or m].
 */
void motor_model_calc_q(const float *shaft_angles, float *q);

/**
 * @brief Calculates the joint velocity qd from the joint position.
 *
 * @note Depending on the number of motors set in the configuration,
 * this function accepts either a scalar or a vector.
 *
 * @param q Pointer to current joint position [rad or m].
 * @param qd Pointer to return the calculated joint velocity [rad/s or m/s].
 */
void motor_model_calc_qd(const float *q, float *qd);

/**
 * @brief Calculates the required motor voltage based on the desired joint force/torque and velocity.
 *
 * The calculation is based on the motor model equations:
 * \f[I = \frac{M}{k_M},\quad M = \frac{Q}{i}\f]
 * \f[U = R\cdot{}I + k_M\cdot{}\omega,\quad \omega = q\cdot{}i\f]
 * Input shaft torque and angular velocity are limited to the maximum values defined in the motor configuration.
 *
 * @note Depending on the number of motors set in the configuration,
 * this function accepts either a scalar or a vector.
 *
 * @param Q Pointer to desired joint force/torque [N or Nm].
 * @param qd Pointer to desired joint velocity [rad/s or m/s].
 * @param U Pointer to return the calculated motor voltage [V].
 */
void motor_model_calc_voltage(const float *Q, const float *qd, float *U);
