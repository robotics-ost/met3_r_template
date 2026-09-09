/**
 * @file sequencer.c
 * @brief Implementation of the sequencer interface.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "sequencer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <safety_system.h>
#include <esp_log.h>

static const char *TAG = "Sequencer"; /**< Tag for logging purposes */

static sequencer_config_t cfg = {0}; /**< Configuration for the sequencer */

static EventGroupHandle_t sequencer_event_group = NULL; /**< Event group for sequencer events */
#define SEQUENCER_INITIALIZED_BIT BIT0                  /**< Bit indicating sequencer is initialized in the event group */

static SemaphoreHandle_t sequencer_mutex = NULL; /**< Mutex for thread-safe access to the sequencer queue */

/**
 * @brief Node structure for the doubly linked list of sequencer steps.
 */
typedef struct doubly_linked_step_list_node
{
    sequencer_step_t step;                     /**< The sequencer step data */
    struct doubly_linked_step_list_node *next; /**< Pointer to the next node in the list */
    struct doubly_linked_step_list_node *prev; /**< Pointer to the previous node in the list */
} doubly_linked_step_list_node_t;

static doubly_linked_step_list_node_t *head = NULL; /**< Head of the doubly linked list */

/**
 * @brief Creates a new node for the doubly linked list with the given sequencer step.
 *
 * @param step Pointer to the sequencer step data.
 *
 * @return Pointer to the newly created node, or NULL if allocation failed.
 */
static doubly_linked_step_list_node_t *create_node(const sequencer_step_t *step)
{
    static uint8_t next_id = 0; // Static variable to keep track of the next ID to assign
    doubly_linked_step_list_node_t *new_node =
        (doubly_linked_step_list_node_t *)pvPortMalloc(sizeof(doubly_linked_step_list_node_t));
    if (new_node == NULL)
    {
        return NULL; // Memory allocation failed
    }
    new_node->step = *step;
    new_node->step.id = next_id++;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

/**
 * @brief Converts a sequencer step type to a human-readable string.
 *
 * @param type The sequencer step type to convert.
 *
 * @return A pointer to the human-readable string representing the step type.
 */
static char *sequencer_step_type_to_string(sequencer_step_type_t type)
{
    switch (type)
    {
    case WAIT:
        return "WAIT";
    default:
        return "UNKNOWN";
    }
}

esp_err_t sequencer_init(const sequencer_config_t *config)
{
    ESP_LOGI(TAG, "Initializing sequencer...");

    // Create sequencer event group if it doesn't exist
    if (sequencer_event_group == NULL)
    {
        sequencer_event_group = xEventGroupCreate();
        if (sequencer_event_group == NULL)
        {
            ESP_LOGE(TAG, "Failed to create sequencer event group");
            return ESP_FAIL;
        }
    }
    else if (xEventGroupGetBits(sequencer_event_group) & SEQUENCER_INITIALIZED_BIT)
    {
        ESP_LOGW(TAG, "Sequencer already initialized.");
        return ESP_OK;
    }

    // Store the configuration
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Invalid configuration: config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    cfg = *config;

    // Validate the config
    if (cfg.task_period_ms == 0)
    {
        ESP_LOGE(TAG, "Invalid configuration: task_period_ms is 0");
        return ESP_ERR_INVALID_ARG;
    }

    // Create a mutex for thread-safe access to the double linked list of sequencer steps
    sequencer_mutex = xSemaphoreCreateMutex();
    if (sequencer_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create sequencer mutex");
        return ESP_FAIL;
    }

    // Initialize the doubly linked list head to NULL
    head = NULL;

    // Set the sequencer initialized bit in the event group
    xEventGroupSetBits(sequencer_event_group, SEQUENCER_INITIALIZED_BIT);
    ESP_LOGI(TAG, "Sequencer initialized.");
    return ESP_OK; // Initialization successful
}

bool sequencer_add_step_to_queue(const sequencer_step_t *step)
{
    if (step == NULL)
    {
        ESP_LOGE(TAG, "Invalid argument: step is NULL");
        return false;
    }

    // Only add the step if the system is not shutting down
    if (safety_system_is_shutting_down())
    {
        return false;
    }

    // Check if the step type is valid
    if (step->type >= _COUNT)
    {
        ESP_LOGW(TAG, "Invalid step type: %d", step->type);
        return false;
    }

    // Acquire the mutex for thread-safe access
    xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
    // Create a new node for the step
    doubly_linked_step_list_node_t *new_node = create_node(step);
    if (new_node == NULL)
    {
        xSemaphoreGive(sequencer_mutex);
        ESP_LOGE(TAG, "Failed to allocate memory for new step node");
        return false;
    }

    if (head == NULL)
    {
        head = new_node; // List is empty, new node becomes the head
    }
    else
    {
        doubly_linked_step_list_node_t *current = head;
        while (current->next != NULL)
        {
            current = current->next; // Traverse to the end of the list
        }
        current->next = new_node; // Append the new node at the end
        new_node->prev = current; // Set the previous pointer of the new node
    }
    xSemaphoreGive(sequencer_mutex);
    ESP_LOGI(TAG, "Added step ID %d to queue: type=%s", new_node->step.id, sequencer_step_type_to_string(step->type));
    return true;
}

bool sequencer_remove_step_from_queue(uint8_t id)
{
    if (head == NULL)
    {
        ESP_LOGW(TAG, "Attempted to remove step from empty queue");
        return false; // List is empty, nothing to remove
    }

    xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
    doubly_linked_step_list_node_t *current = head;
    // Traverse the list to find the node with the matching ID
    while (current != NULL && current->step.id != id)
    {
        current = current->next;
    }

    // If we reached the end of the list without finding the ID, return false
    if (current == NULL)
    {
        xSemaphoreGive(sequencer_mutex);
        ESP_LOGW(TAG, "Step ID %d not found in queue", id);
        return false; // ID not found in the list
    }

    // Remove the node from the list
    if (current->prev != NULL)
    {
        current->prev->next = current->next;
    }
    else
    {
        head = current->next; // Removing the head node
    }

    if (current->next != NULL)
    {
        current->next->prev = current->prev;
    }

    // Free the memory allocated for the node
    vPortFree(current);
    xSemaphoreGive(sequencer_mutex);
    ESP_LOGI(TAG, "Removed step ID %d from queue", id);
    return true;
}

void sequencer_get_first_N_steps_in_queue(const uint8_t N, sequencer_step_t steps[N], uint8_t *count)
{
    // Validate input parameters
    if (N == 0)
    {
        ESP_LOGE(TAG, "Invalid argument: N is 0");
        return;
    }
    if (steps == NULL || count == NULL)
    {
        ESP_LOGE(TAG, "Invalid argument: steps or count is NULL");
        return;
    }
    *count = 0;

    xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
    doubly_linked_step_list_node_t *current = head;
    while ((*count) < N && current != NULL)
    {
        steps[(*count)++] = current->step; // Copy the step data to the output array
        current = current->next;           // Move to the next node
    }
    xSemaphoreGive(sequencer_mutex);
}

void sequencer_task(void *arg)
{
    // Register the sequencer task with the safety system
    ESP_ERROR_CHECK(safety_system_register_task());

    // Wait for the sequencer to be initialized before starting the main loop
    xEventGroupWaitBits(sequencer_event_group, SEQUENCER_INITIALIZED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Main loop for processing sequencer steps
    TickType_t last_wake = xTaskGetTickCount();
    static sequencer_step_t current_step = {0};
    static bool step_in_progress = false;
    static bool step_initialized = false;
    static bool abort_requested = false;
    while (!safety_system_is_shutting_down())
    {
        // If no step is currently in progress, check if there are steps in the queue
        if (!step_in_progress)
        {
            xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
            bool has_step = (head != NULL);
            if (has_step)
            {
                current_step = head->step;
            }
            xSemaphoreGive(sequencer_mutex);
            if (has_step)
            {
                ESP_LOGI(TAG, "Starting step ID %d of type %s", current_step.id, sequencer_step_type_to_string(current_step.type));
                step_in_progress = true;
                step_initialized = false;
                abort_requested = false;
            }
        }
        else if (step_in_progress)
        {
            // Check if the step got removed from the queue while it was being executed
            xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
            bool removed = (head == NULL || head->step.id != current_step.id);
            xSemaphoreGive(sequencer_mutex);
            if (removed)
            {
                ESP_LOGW(TAG, "Step ID %d was removed from the queue during execution, aborting...", current_step.id);
                abort_requested = true;
            }

            // Handle the step based on its type
            switch (current_step.type)
            {
            case WAIT:
                if (current_step.wait.duration_ms == 0)
                {
                    ESP_LOGW(TAG, "WAIT step with zero duration, removing.");
                    sequencer_remove_step_from_queue(current_step.id);
                    step_in_progress = false;
                    step_initialized = false;
                    abort_requested = false;
                    break;
                }
                // Initialize the wait step on the first cycle
                static uint32_t i = 0;
                if (!step_initialized)
                {
                    ESP_LOGI(TAG, "Executing WAIT step for %u ms", current_step.wait.duration_ms);
                    i = 0;
                    if (current_step.wait.duration_ms % cfg.task_period_ms != 0)
                    {
                        // If the wait duration is not a multiple of the task period, we need to wait for the remainder
                        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(current_step.wait.duration_ms % cfg.task_period_ms));
                    }
                    step_initialized = true;
                }
                // Check if the wait duration has been reached
                if (i >= (current_step.wait.duration_ms / cfg.task_period_ms))
                {
                    ESP_LOGI(TAG, "WAIT step completed.");
                    step_in_progress = false;
                    step_initialized = false;
                    abort_requested = false;
                    sequencer_remove_step_from_queue(current_step.id); // Remove the completed step from the queue
                    break;
                }
                // Check if an abort has been requested
                if (abort_requested)
                {
                    step_in_progress = false;
                    step_initialized = false;
                    abort_requested = false;
                    ESP_LOGW(TAG, "WAIT step aborted.");
                    break;
                }
                // Increment the cycle counter
                i++;
                break;
            default:
                ESP_LOGW(TAG, "Unknown step type: %d", current_step.type);
                step_in_progress = false;
                step_initialized = false;
                abort_requested = false;
                sequencer_remove_step_from_queue(current_step.id); // Remove the unknown step from the queue
                break;
            }
        }
        else
        {
            // No step in progress and no steps in the queue, do nothing
        }

        // Delay until the next cycle based on the configured task period
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.task_period_ms));
    }

    // Clean up
    xEventGroupClearBits(sequencer_event_group, SEQUENCER_INITIALIZED_BIT);
    while (true)
    {
        xSemaphoreTake(sequencer_mutex, portMAX_DELAY);
        if (head == NULL)
        {
            xSemaphoreGive(sequencer_mutex);
            break;
        }
        uint8_t id = head->step.id;
        xSemaphoreGive(sequencer_mutex);
        sequencer_remove_step_from_queue(id);
    }
    ESP_ERROR_CHECK(safety_system_unregister_task());
    ESP_LOGI(TAG, "Sequencer task finished.");
    vTaskDelete(NULL);
}
