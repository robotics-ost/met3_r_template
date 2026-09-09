/**
 * @file safety_system.h
 * @brief Safety system interface for the application.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>
#include <stdint.h>

/**
 * @brief Safety system configuration structure
 */
typedef struct
{
    uint32_t task_period_ms;         /**< Safety system task period in milliseconds */
    unsigned int event_queue_length; /**< Length of the safety event queue */
} safety_system_config_t;

/**
 * @brief Safety Levels
 *
 * This enumeration defines the various safety levels of the system.
 */
typedef enum : uint8_t
{
    SL_OFF = 0,       /**< System is off */
    SL_INITIALIZING,  /**< System is initializing */
    SL_SHUTTING_DOWN, /**< System is shutting down */
    SL_EMERGENCY,     /**< Emergency state triggered */
    SL_READY,         /**< System is ready for operation */
    SL_ACTIVE,        /**< System is actively operating */
} safety_level_t;

/**
 * @brief Safety Events
 *
 * This enumeration defines the various events that can trigger transitions between safety levels.
 */
typedef enum
{
    SE_DO_SYSTEM_ON = 0,         /**< Event to turn the system on */
    SE_DO_SYSTEM_OFF,            /**< Event to turn the system off */
    SE_SYSTEM_INITIALIZED,       /**< Event indicating system initialization complete */
    SE_SYSTEM_SHUTDOWN_COMPLETE, /**< Event indicating system shutdown complete */
    SE_EMERGENCY_TRIGGERED,      /**< Event indicating an emergency has been triggered */
    SE_EMERGENCY_CLEARED,        /**< Event indicating an emergency has been cleared */
    SE_START_OPERATION,          /**< Event to start system operation */
    SE_STOP_OPERATION,           /**< Event to stop system operation */
} safety_event_t;

/**
 * @brief Initialize the safety system.
 *
 * This function initializes the safety system, including creating the safety event queue.
 *
 * @param config Pointer to the safety system configuration structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t safety_system_init(const safety_system_config_t *config);

/**
 * @brief Get the current safety level.
 * @return The current safety level.
 */
safety_level_t safety_system_get_level(void);

/**
 * @brief Register a safety event.
 *
 * This function adds a safety event to the safety event queue for processing.
 *
 * @param event The safety event to register.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t safety_system_register_event(safety_event_t event);

/**
 * @brief Register a task with the safety system.
 *
 * This function must be called by all tasks except the safety system task itself to ensure they are properly managed during shutdown.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t safety_system_register_task(void);

/**
 * @brief Unregister a task from the safety system.
 *
 * This function must be called by tasks before they exit to ensure proper cleanup and shutdown signaling.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t safety_system_unregister_task(void);

/**
 * @brief Check if the safety system is shutting down.
 *
 * This function can be called by tasks to determine if the system is in the process of shutting down, allowing them to perform any necessary cleanup before exiting.
 *
 * @return true if the system is shutting down, false otherwise.
 */
bool safety_system_is_shutting_down(void);

/**
 * @brief Safety system task.
 *
 * This function implements the main task for the safety system.
 * It processes safety events from the queue, manages state transitions, and ensures the system operates within defined safety levels.
 *
 * @param pvParameters Pointer to task parameters.
 */
void safety_system_task(void *pvParameters);
