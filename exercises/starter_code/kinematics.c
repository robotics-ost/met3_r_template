/**
 * @file kinematics.c
 * @brief Kinematics implementation for the robot system.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "kinematics.h"
#include "common_math.h"
#include <math.h>

static kinematics_config_t cfg = {0}; /**< Configuration for the kinematics module */

esp_err_t kinematics_init(const kinematics_config_t *config)
{
    // Store the configuration
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    cfg = *config;

    // Validate the configuration parameters
    if (cfg.dt <= 0.0f ||
        cfg.wheel_distance <= 0.0f)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

void kinematics_calc_fw_kin_odom(const float qd[2], float xi[3], float xid[3])
{
    /* TO DO */
}

void kinematics_calc_inv_kin(const float xid_d[3], float qd_d[2])
{
    /* TO DO */
}
