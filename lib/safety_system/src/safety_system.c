/**
 * @file safety_system.c
 * @brief Safety system implementation.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "safety_system.h"
#include "motor_driver.h"
#include "udp_interface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <stdatomic.h>

static const char *TAG = "Safety System"; /**< Tag for logging purposes */

static safety_system_config_t cfg = {0}; /**< Configuration for the safety system */

static safety_level_t current_safety_level = SL_INITIALIZING; /**< Current safety level of the system */

static QueueHandle_t safety_event_queue = NULL; /**< Queue for safety events */

static EventGroupHandle_t safety_event_group = NULL; /**< Event group for safety system events */
#define SAFETY_SYSTEM_INITIALIZED_BIT BIT0           /**< Bit indicating safety system is initialized in the event group */
#define SHUTTING_DOWN_BIT BIT1                       /**< Bit indicating system is shutting down in the event group */

static atomic_uint_fast8_t registered_task_count = 0;       /**< Count of tasks registered with the safety system */
static SemaphoreHandle_t xAllTasksShutdownSemaphore = NULL; /**< Semaphore to signal all tasks to shutdown */

/**
 * @brief Convert safety level to string representation.
 *
 * @param level The safety level to convert.
 *
 * @return A string representation of the safety level.
 */
static const char *safety_level_to_string(safety_level_t level)
{
    switch (level)
    {
    case SL_OFF:
        return "SL_OFF";
    case SL_INITIALIZING:
        return "SL_INITIALIZING";
    case SL_SHUTTING_DOWN:
        return "SL_SHUTTING_DOWN";
    case SL_EMERGENCY:
        return "SL_EMERGENCY";
    case SL_READY:
        return "SL_READY";
    case SL_ACTIVE:
        return "SL_ACTIVE";
    default:
        return "UNKNOWN_LEVEL";
    }
}

/**
 * @brief Convert safety event to string representation.
 *
 * @param event The safety event to convert.
 *
 * @return A string representation of the safety event.
 */
static const char *safety_event_to_string(safety_event_t event)
{
    switch (event)
    {
    case SE_DO_SYSTEM_ON:
        return "SE_DO_SYSTEM_ON";
    case SE_DO_SYSTEM_OFF:
        return "SE_DO_SYSTEM_OFF";
    case SE_SYSTEM_INITIALIZED:
        return "SE_SYSTEM_INITIALIZED";
    case SE_SYSTEM_SHUTDOWN_COMPLETE:
        return "SE_SYSTEM_SHUTDOWN_COMPLETE";
    case SE_EMERGENCY_TRIGGERED:
        return "SE_EMERGENCY_TRIGGERED";
    case SE_EMERGENCY_CLEARED:
        return "SE_EMERGENCY_CLEARED";
    case SE_START_OPERATION:
        return "SE_START_OPERATION";
    case SE_STOP_OPERATION:
        return "SE_STOP_OPERATION";
    default:
        return "UNKNOWN_EVENT";
    }
}

esp_err_t safety_system_init(const safety_system_config_t *config)
{
    ESP_LOGI(TAG, "Initializing safety system...");
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Create safety system event group if it doesn't exist
    if (safety_event_group == NULL)
    {
        safety_event_group = xEventGroupCreate();
        if (safety_event_group == NULL)
        {
            ESP_LOGE(TAG, "Failed to create safety system event group");
            return ESP_FAIL;
        }
    }
    else if (xEventGroupGetBits(safety_event_group) & SAFETY_SYSTEM_INITIALIZED_BIT)
    {
        ESP_LOGW(TAG, "Safety system already initialized.");
        return ESP_OK;
    }

    // Clear the SHUTTING_DOWN_BIT in the event group
    xEventGroupClearBits(safety_event_group, SHUTTING_DOWN_BIT);

    // Store the configuration
    cfg = *config;

    // Initialize the safety event queue if it doesn't exist
    if (safety_event_queue == NULL)
    {
        safety_event_queue = xQueueCreate(cfg.event_queue_length, sizeof(safety_event_t));
        if (safety_event_queue == NULL)
        {
            ESP_LOGE(TAG, "Failed to create safety event queue");
            return ESP_FAIL;
        }
    }

    // Initialize the semaphore for task shutdown signaling
    xAllTasksShutdownSemaphore = xSemaphoreCreateBinary();
    if (xAllTasksShutdownSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create shutdown semaphore");
        return ESP_FAIL;
    }

    // Set the safety system initialized bit in the event group
    xEventGroupSetBits(safety_event_group, SAFETY_SYSTEM_INITIALIZED_BIT);
    ESP_LOGI(TAG, "Safety system initialized.");
    return ESP_OK;
}

safety_level_t safety_system_get_level(void)
{
    return current_safety_level;
}

esp_err_t safety_system_register_event(safety_event_t event)
{
    if (event == SE_EMERGENCY_TRIGGERED)
    {
        xQueueReset(safety_event_queue); // Clear the queue to prioritize emergency events
    }
    if (xQueueSend(safety_event_queue, &event, 0) != pdPASS)
    {
        return ESP_FAIL; // Queue full
    }
    return ESP_OK;
}

esp_err_t safety_system_register_task(void)
{
    atomic_fetch_add(&registered_task_count, 1);
    return ESP_OK;
}

esp_err_t safety_system_unregister_task(void)
{
    if (atomic_fetch_sub(&registered_task_count, 1) == 1)
    {
        xSemaphoreGive(xAllTasksShutdownSemaphore);
    }
    return ESP_OK;
}

bool safety_system_is_shutting_down(void)
{
    return (xEventGroupGetBits(safety_event_group) & SHUTTING_DOWN_BIT);
}

/**
 * @brief Handle a safety event.
 *
 * This function processes a safety event based on the current safety level.
 *
 * @param event The safety event to handle.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
static esp_err_t safety_system_handle_event(safety_event_t event)
{
    esp_err_t ret = ESP_FAIL;
    safety_level_t previous_safety_level = current_safety_level;
    switch (current_safety_level)
    {
    case SL_OFF:
        switch (event)
        {
        case SE_DO_SYSTEM_ON:
            current_safety_level = SL_INITIALIZING;
            ret = ESP_OK;
            break;
        case SE_EMERGENCY_TRIGGERED:
            current_safety_level = SL_EMERGENCY;
            ret = ESP_OK;
            break;
        default:
            break;
        }
        break;
    case SL_INITIALIZING:
        switch (event)
        {
        case SE_SYSTEM_INITIALIZED:
            current_safety_level = SL_READY;
            ret = ESP_OK;
            break;
        case SE_EMERGENCY_TRIGGERED:
            current_safety_level = SL_EMERGENCY;
            ret = ESP_OK;
            break;
        case SE_DO_SYSTEM_OFF:
            current_safety_level = SL_SHUTTING_DOWN;
            ret = ESP_OK;
            break;
        default:
            break;
        }
        break;
    case SL_SHUTTING_DOWN:
        switch (event)
        {
        case SE_SYSTEM_SHUTDOWN_COMPLETE:
            current_safety_level = SL_OFF;
            ret = ESP_OK;
            break;
        default:
            break;
        }
        break;
    case SL_EMERGENCY:
        switch (event)
        {
        case SE_EMERGENCY_CLEARED:
            current_safety_level = SL_READY;
            ret = ESP_OK;
            break;
        case SE_DO_SYSTEM_OFF:
            current_safety_level = SL_SHUTTING_DOWN;
            ret = ESP_OK;
            break;
        default:
            break;
        }
        break;
    case SL_READY:
        switch (event)
        {
        case SE_START_OPERATION:
            current_safety_level = SL_ACTIVE;
            ret = ESP_OK;
            break;
        case SE_EMERGENCY_TRIGGERED:
            current_safety_level = SL_EMERGENCY;
            ret = ESP_OK;
            break;
        case SE_DO_SYSTEM_OFF:
            current_safety_level = SL_SHUTTING_DOWN;
            ret = ESP_OK;
            break;
        default:
            break;
        }
        break;
    case SL_ACTIVE:
    {
        switch (event)
        {
        case SE_STOP_OPERATION:
            current_safety_level = SL_READY;
            ret = ESP_OK;
            break;
        case SE_EMERGENCY_TRIGGERED:
            current_safety_level = SL_EMERGENCY;
            ret = ESP_OK;
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    if (ret == ESP_FAIL)
    {
        ESP_LOGW(TAG, "No transition defined for event: %s in safety level: %s", safety_event_to_string(event), safety_level_to_string(previous_safety_level));
    }
    else
    {
        ESP_LOGI(TAG, "Transitioned from safety level: %s to safety level: %s due to event: %s", safety_level_to_string(previous_safety_level), safety_level_to_string(current_safety_level), safety_event_to_string(event));
    }
    return ret;
}

void safety_system_task(void *pvParameters)
{
    // Wait for the safety system to be initialized before starting the main loop
    xEventGroupWaitBits(safety_event_group, SAFETY_SYSTEM_INITIALIZED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Safety system main loop
    safety_event_t event;
    TickType_t last_wake = xTaskGetTickCount();
    while (1)
    {
        // Handle safety events
        if (xQueueReceive(safety_event_queue, &event, 0) == pdTRUE)
        {
            safety_system_handle_event(event);
        }

        // Safety level actions
        switch (current_safety_level)
        {
        case SL_OFF:
            motor_driver_set_state(MOTORS_DISABLED);
            // Enter deep sleep mode
            esp_deep_sleep_start();
            break;
        case SL_INITIALIZING:
            motor_driver_set_state(MOTORS_DISABLED);
            break;
        case SL_SHUTTING_DOWN:
            motor_driver_set_state(MOTORS_DISABLED);
            // Signal all tasks to shutdown
            xEventGroupSetBits(safety_event_group, SHUTTING_DOWN_BIT);
            // Wait for all tasks to acknowledge shutdown
            xSemaphoreTake(xAllTasksShutdownSemaphore, portMAX_DELAY);
            // Trigger the system shutdown complete event
            vTaskDelay(pdMS_TO_TICKS(50));
            safety_system_register_event(SE_SYSTEM_SHUTDOWN_COMPLETE);
            break;
        case SL_EMERGENCY:
            motor_driver_set_state(MOTORS_DISABLED);
            break;
        case SL_READY:
            motor_driver_set_state(MOTORS_DISABLED);
            break;
        case SL_ACTIVE:
            motor_driver_set_state(MOTORS_ENABLED);
            break;
        default:
            break;
        }

        // Add the current safety level to the UDP telemetry samples queue
        udp_interface_add_safety_sample(current_safety_level);

        // Wait for the next cycle
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.task_period_ms));
    }
}
