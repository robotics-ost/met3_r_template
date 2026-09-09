/**
 * @file encoder_driver.c
 * @brief Encoder driver implementation.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "encoder_driver.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <driver/pulse_cnt.h>
#include <math.h>

#define ENCODER_A_C1 34 /**< Encoder A Channel 1 */
#define ENCODER_A_C2 35 /**< Encoder A Channel 2 */
#define ENCODER_B_C1 27 /**< Encoder B Channel 1 */
#define ENCODER_B_C2 16 /**< Encoder B Channel 2 */

#define ENCODER_PCNT_HIGH_LIMIT 32767   /**< PCNT high watch/overflow limit (INT16_MAX) */
#define ENCODER_PCNT_LOW_LIMIT (-32767) /**< PCNT low watch/overflow limit */

static const char *TAG = "Encoder Driver"; /**< Tag for logging purposes */

static pcnt_unit_handle_t encoder_A_handle = NULL; /**< Handle for encoder A */
static pcnt_unit_handle_t encoder_B_handle = NULL; /**< Handle for encoder B */

static encoder_driver_config_t cfg = {0}; /**< Configuration for the encoder driver */

static EventGroupHandle_t encoder_driver_event_group = NULL; /**< Event group for control system events */
#define ENCODER_DRIVER_INITIALIZED_BIT BIT0                  /**< Bit indicating encoder driver is initialized in the event group */

/**
 * @brief Helper function to initialize an encoder unit
 *
 * @param encoder_handle Pointer to the encoder unit handle
 * @param C1 GPIO pin for channel 1
 * @param C2 GPIO pin for channel 2
 */
static void encoder_driver_init_helper(pcnt_unit_handle_t *encoder_handle, int C1, int C2)
{
    // Create a new PCNT unit for the encoder, with accum_count enabled so
    // pcnt_unit_get_count() transparently compensates for hardware
    // overflow at +/-ENCODER_PCNT_*_LIMIT
    pcnt_unit_config_t pcnt_unit_config = {
        .high_limit = ENCODER_PCNT_HIGH_LIMIT,
        .low_limit = ENCODER_PCNT_LOW_LIMIT,
        .flags.accum_count = 1,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&pcnt_unit_config, encoder_handle));

    // Configure the glitch filter for the encoder unit
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(*encoder_handle, &filter_config));

    // Configure the channels for the encoder unit
    pcnt_chan_config_t channel_A_config = {
        .edge_gpio_num = C1,
        .level_gpio_num = C2,
    };
    pcnt_channel_handle_t channel_A_handle = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(*encoder_handle, &channel_A_config, &channel_A_handle));
    pcnt_chan_config_t channel_B_config = {
        .edge_gpio_num = C2,
        .level_gpio_num = C1,
    };
    pcnt_channel_handle_t channel_B_handle = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(*encoder_handle, &channel_B_config, &channel_B_handle));

    // Set the edge and level actions for the channels
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channel_A_handle, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(channel_A_handle, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channel_B_handle, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(channel_B_handle, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // Add watch points at the counter limits - required for accum_count to
    // compensate overflow, per the ESP-IDF "Compensate Overflow Loss" guide.
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(*encoder_handle, ENCODER_PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(*encoder_handle, ENCODER_PCNT_LOW_LIMIT));

    // Enable and start the encoder unit
    ESP_ERROR_CHECK(pcnt_unit_enable(*encoder_handle));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(*encoder_handle));
    ESP_ERROR_CHECK(pcnt_unit_start(*encoder_handle));
}

esp_err_t encoder_driver_init(const encoder_driver_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Configuring Encoder Driver...");

    // Create encoder driver event group if it doesn't exist
    if (encoder_driver_event_group == NULL)
    {
        encoder_driver_event_group = xEventGroupCreate();
        if (encoder_driver_event_group == NULL)
        {
            ESP_LOGE(TAG, "Failed to create encoder driver event group");
            return ESP_FAIL;
        }
    }
    else if (xEventGroupGetBits(encoder_driver_event_group) & ENCODER_DRIVER_INITIALIZED_BIT)
    {
        ESP_LOGW(TAG, "Encoder driver already initialized.");
        return ESP_FAIL;
    }

    // Store the configuration
    cfg = *config;

    // Initialize encoder A and B
    encoder_driver_init_helper(&encoder_A_handle, ENCODER_A_C1, ENCODER_A_C2);
    encoder_driver_init_helper(&encoder_B_handle, ENCODER_B_C1, ENCODER_B_C2);

    // Set the encoder driver initialized bit in the event group
    xEventGroupSetBits(encoder_driver_event_group, ENCODER_DRIVER_INITIALIZED_BIT);
    ESP_LOGI(TAG, "Encoder driver initialized.");
    return ESP_OK;
}

/**
 * @brief Check whether the encoder driver has completed initialization.
 */
static bool encoder_driver_is_initialized(void)
{
    return encoder_driver_event_group != NULL &&
           (xEventGroupGetBits(encoder_driver_event_group) & ENCODER_DRIVER_INITIALIZED_BIT);
}

static void encoder_driver_get_angle_helper(pcnt_unit_handle_t encoder_handle, float pulses_per_revolution, float *angle)
{
    int pulse_count = 0;

    // Get the pulse count from the encoder
    ESP_ERROR_CHECK(pcnt_unit_get_count(encoder_handle, &pulse_count));

    // Calculate the shaft angle in radians based on the pulse count and pulses per revolution
    // angle = pulse_count / 4 / pulses_per_revolution * 2 * pi
    //       = pulse_count / pulses_per_revolution * pi / 2
    *angle = (float)pulse_count / pulses_per_revolution * M_PI_2;
}

void encoder_driver_get_angle_A(float *angle)
{
    if (!encoder_driver_is_initialized())
    {
        ESP_LOGE(TAG, "Encoder driver not initialized");
        *angle = 0.0f;
        return;
    }
    encoder_driver_get_angle_helper(encoder_A_handle, cfg.enc_A_pulses_per_revolution, angle);
}

void encoder_driver_get_angle_B(float *angle)
{
    if (!encoder_driver_is_initialized())
    {
        ESP_LOGE(TAG, "Encoder driver not initialized");
        *angle = 0.0f;
        return;
    }
    encoder_driver_get_angle_helper(encoder_B_handle, cfg.enc_B_pulses_per_revolution, angle);
}

void encoder_driver_get_angles(float angles[2])
{
    encoder_driver_get_angle_A(&angles[0]);
    encoder_driver_get_angle_B(&angles[1]);
}
