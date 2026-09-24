/**
 * @file motor_model.c
 * @brief Motor model implementation file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "motor_model.h"
#include "common_math.h"
#include <stdlib.h>
#include <string.h>

static motor_model_config_t cfg = {0}; /**< Configuration for the motor model */

static float *q_prev = NULL; /**< Pointer to the previous joint position for velocity calculation */

esp_err_t motor_model_init(const motor_model_config_t *config)
{
    // Store the configuration
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    cfg = *config;

    // Validate the configuration parameters
    if (cfg.dt <= 0.0f ||
        cfg.params == NULL ||
        cfg.nr_of_motors <= 0)
    {
        return ESP_ERR_INVALID_ARG;
    }
    for (uint8_t i = 0; i < cfg.nr_of_motors; i++)
    {
        if (cfg.params[i].transmission_ratio <= 0.0f ||
            cfg.params[i].armature_resistance <= 0.0f ||
            cfg.params[i].motor_constant <= 0.0f ||
            cfg.params[i].max_torque_intermittent <= 0.0f ||
            cfg.params[i].max_velocity <= 0.0f)
        {
            return ESP_FAIL;
        }
    }

    // Allocate memory for the previous joint positions based on the number of motors
    q_prev = malloc(sizeof(float) * cfg.nr_of_motors);
    if (q_prev == NULL)
    {
        return ESP_FAIL;
    }

    // Initialize the previous joint positions to zero
    memset(q_prev, 0, sizeof(float) * cfg.nr_of_motors);

    return ESP_OK;
}

void motor_model_cleanup(void)
{
    if (q_prev != NULL)
    {
        free(q_prev);
        q_prev = NULL;
    }
}

void motor_model_calc_q(const float *shaft_angles, float *q)
{
    /* TO DO */
}

void motor_model_calc_qd(const float *q, float *qd)
{
    /* TO DO */
}

void motor_model_calc_voltage(const float *Q, const float *qd, float *U)
{
    /* TO DO */
}
