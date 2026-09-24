/**
 * @file position_controller_diff_drive.c
 * @brief Cartesian position controller for differential drive robots implementation file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "position_controller_diff_drive.h"
#include "common_math.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <math.h>

static position_controller_config_t cfg = {0}; /**< Configuration for the position controller */

static EventGroupHandle_t event_group = NULL; /**< Event group for position controller events */
#define TARGET_REACHED_BIT BIT0               /**< Bit for target reached event */
#define TARGET_ABORTED_BIT BIT1               /**< Bit for target aborted event */

static float _xi_d[3] = {0.0f, 0.0f, 0.0f};  /**< Desired position and orientation in the global frame [x, y, phi] */
static SemaphoreHandle_t _xi_d_mutex = NULL; /**< Mutex for protecting access to shared desired position and orientation */

esp_err_t position_controller_init(const position_controller_config_t *config)
{
    // Store the configuration
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    cfg = *config;

    // Validate the configuration parameters
    if (cfg.position_tolerance <= 0.0f)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Create an event group for position controller events
    event_group = xEventGroupCreate();
    if (event_group == NULL)
    {
        return ESP_FAIL;
    }

    // Set the initial state of the event group
    xEventGroupSetBits(event_group, TARGET_REACHED_BIT);
    xEventGroupClearBits(event_group, TARGET_ABORTED_BIT);

    // Create a mutex for protecting access to shared desired position and orientation
    _xi_d_mutex = xSemaphoreCreateMutex();
    if (_xi_d_mutex == NULL)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

bool position_controller_set_target(const float xi_d[3])
{
    if (xi_d == NULL)
    {
        return false;
    }

    // Update the desired position and orientation
    xSemaphoreTake(_xi_d_mutex, portMAX_DELAY);
    for (int i = 0; i < 3; i++)
    {
        _xi_d[i] = xi_d[i];
    }
    xSemaphoreGive(_xi_d_mutex);

    // Clear the target reached bit
    xEventGroupClearBits(event_group, TARGET_REACHED_BIT);
    return true;
}

bool position_controller_is_target_reached(void)
{
    return (xEventGroupGetBits(event_group) & TARGET_REACHED_BIT) != 0;
}

void position_controller_abort(void)
{
    // Set the target aborted bit
    xEventGroupSetBits(event_group, TARGET_ABORTED_BIT);
}

void position_controller_run(const float xi[3], float xid_d[3])
{
    /* TO DO */
}
