/**
 * @file sequencer.h
 * @brief Sequencer interface for the application.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
/**
 * @brief Configuration structure for the sequencer.
 */
typedef struct
{
    uint32_t task_period_ms; /**< Sequencer task period [ms] */
} sequencer_config_t;

/**
 * @brief Enumeration of sequencer step types.
 */
typedef enum
{
    WAIT = 0,             /**< Wait for a specified duration */

    _COUNT /**< Count of step types, used for validation */
} sequencer_step_type_t;

/**
 * @brief Structure representing a sequencer step.
 */
typedef struct
{
    sequencer_step_type_t type; /**< Step type */
    uint8_t id;                 /**< Unique identifier for the step (wraps at 255) */
    union
    {
        struct
        {
            uint32_t duration_ms; /**< Duration to wait in milliseconds */
        } wait;                   /**< Parameters for WAIT step */
    };
} sequencer_step_t;

/**
 * @brief Initializes the sequencer with the specified configuration.
 *
 * @param config Pointer to the sequencer configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sequencer_init(const sequencer_config_t *config);

/**
 * @brief Adds a sequencer step to the queue.
 *
 * @param step Pointer to the sequencer step to be added.
 *
 * @return true if the step was successfully added, false otherwise.
 */
bool sequencer_add_step_to_queue(const sequencer_step_t *step);

/**
 * @brief Removes a sequencer step from the queue.
 *
 * @param id ID of the step to be removed.
 *
 * @return true if the step was successfully removed, false otherwise.
 */
bool sequencer_remove_step_from_queue(uint8_t id);

/**
 * @brief Retrieves the first N sequencer steps currently in the queue.
 *
 * @param N The maximum number of steps to retrieve.
 * @param steps Array to store the retrieved steps.
 * @param count Number of steps retrieved.
 */
void sequencer_get_first_N_steps_in_queue(const uint8_t N, sequencer_step_t steps[N], uint8_t *count);

/**
 * @brief The main task function for the sequencer.
 *
 * This function runs in a FreeRTOS task and processes the sequencer steps in the queue.
 * It handles the execution of MOVE_TO_POSE and WAIT steps based on their types.
 *
 * @param arg Pointer to any arguments passed to the task (not used).
 */
void sequencer_task(void *arg);
