/**
 * @file pd_controller.c
 * @brief PD position controller implementation file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "pd_controller.h"
#include "common_math.h"
#include <stdlib.h>
#include <string.h>

static pd_controller_config_t cfg = {0}; /**< Configuration for the PD position controller */

static float *e_prev = NULL; /**< Pointer to store the previous position errors */

esp_err_t pd_controller_init(const pd_controller_config_t *config)
{
    // Store the configuration
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    cfg = *config;

    // Validate the configuration parameters
    if (cfg.dt <= 0.0f ||
        cfg.kp < 0.0f ||
        cfg.kd < 0.0f ||
        cfg.M == NULL ||
        cfg.dim <= 0)
    {
        return ESP_ERR_INVALID_ARG;
    }
    for (uint8_t i = 0; i < cfg.dim; i++)
    {
        if (cfg.M[i * cfg.dim + i] <= 0.0f) // Check diagonal elements of the mass matrix
        {
            return ESP_FAIL;
        }
    }

    // Allocate memory for the previous position errors
    e_prev = malloc(sizeof(float) * cfg.dim);
    if (e_prev == NULL)
    {
        return ESP_FAIL;
    }

    // Initialize the previous position errors to zero
    memset(e_prev, 0, sizeof(float) * cfg.dim);

    return ESP_OK;
}

void pd_controller_cleanup(void)
{
    if (e_prev != NULL)
    {
        free(e_prev);
        e_prev = NULL;
    }
}

void pd_controller_calc_Q(const float *q, const float *q_d, float *Q)
{
    /* TO DO */
}
