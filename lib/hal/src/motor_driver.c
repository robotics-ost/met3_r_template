/**
 * @file motor_driver.c
 * @brief Motor driver implementation.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "motor_driver.h"
#include "safety_system.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/i2c_master.h>
#include <math.h>

#define MOTOR_A_IN1 17 /**< GPIO pin for motor A IN1 */
#define MOTOR_A_IN2 21 /**< GPIO pin for motor A IN2 */
#define MOTOR_A_PWM 25 /**< GPIO pin for motor A PWM */
#define MOTOR_B_IN1 22 /**< GPIO pin for motor B IN1 */
#define MOTOR_B_IN2 23 /**< GPIO pin for motor B IN2 */
#define MOTOR_B_PWM 26 /**< GPIO pin for motor B PWM */

#define MOTOR_PWM_FREQUENCY 20000              /**< PWM frequency for motor control in Hz */
#define MOTOR_PWM_RESOLUTION LEDC_TIMER_10_BIT /**< PWM resolution for motor control */

static const char *TAG = "Motor Driver"; /**< Tag for logging purposes */

static motor_driver_config_t cfg = {0}; /**< Configuration for the motor driver */

static i2c_master_dev_handle_t ina219_handle; /**< Handle for the INA219 I2C device */

static float bus_voltage = 0.0f; /**< Current bus voltage reading */

static motor_driver_state_t motor_driver_state; /**< Current state of the motor driver */

static EventGroupHandle_t motor_driver_event_group = NULL; /**< Event group for motor driver events */
#define MOTOR_DRIVER_INITIALIZED_BIT BIT0                  /**< Bit indicating motor driver is initialized in the event group */

/**
 * @brief Update the bus voltage reading from the INA219 I2C device.
 */
void motor_driver_update_bus_voltage(void)
{
    uint8_t write_buf[1] = {0x02};
    uint8_t read_buf[2] = {0};

    i2c_master_transmit_receive(ina219_handle, write_buf, sizeof(write_buf), read_buf, sizeof(read_buf), 20);
    bus_voltage = (float)(read_buf[0] << 5 | read_buf[1] >> 3) / 1000.0f * 4.0f; // LSB = 4mV
    if (bus_voltage < cfg.min_bus_voltage)
    {
        ESP_LOGW(TAG, "Bus voltage too low: %.2f V", bus_voltage);
        ESP_LOGW(TAG, "Check power supply and connections.");
    }
}

esp_err_t motor_driver_init(const motor_driver_config_t *config)
{
    ESP_LOGI(TAG, "Configuring Motor Driver...");
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Create motor driver event group if it doesn't exist
    if (motor_driver_event_group == NULL)
    {
        motor_driver_event_group = xEventGroupCreate();
        if (motor_driver_event_group == NULL)
        {
            ESP_LOGE(TAG, "Failed to create motor driver event group");
            return ESP_FAIL;
        }
    }
    else if (xEventGroupGetBits(motor_driver_event_group) & MOTOR_DRIVER_INITIALIZED_BIT)
    {
        ESP_LOGW(TAG, "Motor driver already initialized.");
        return ESP_FAIL;
    }

    // Store the configuration
    cfg = *config;

    // Configure GPIO pins for both motors
    gpio_config_t motors_gpio_config = {
        .pin_bit_mask = ((1ULL << MOTOR_A_IN1) |
                         (1ULL << MOTOR_A_IN2) |
                         (1ULL << MOTOR_B_IN1) |
                         (1ULL << MOTOR_B_IN2)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&motors_gpio_config));

    // Set initial drive states to "Stop"
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_A_IN1, 0));
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_A_IN2, 0));
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_B_IN1, 0));
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_B_IN2, 0));
    motor_driver_state = MOTORS_DISABLED;

    // Configure PWM timer for both motors
    ledc_timer_config_t pwm_timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = MOTOR_PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = MOTOR_PWM_FREQUENCY,
        .clk_cfg = LEDC_USE_APB_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer_config));

    // Configure PWM channel for motor A
    ledc_channel_config_t motor_A_pwm_channel_config = {
        .gpio_num = MOTOR_A_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor_A_pwm_channel_config));

    // Configure PWM channel for motor B
    ledc_channel_config_t motor_B_pwm_channel_config = {
        .gpio_num = MOTOR_B_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor_B_pwm_channel_config));

    // Initialize INA219 I2C device to read bus voltage
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(I2C_NUM_0, &bus_handle));
    i2c_device_config_t ina219_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = 0x42,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &ina219_cfg, &ina219_handle));

    // Update bus voltage
    motor_driver_update_bus_voltage();

    // Set the motor driver initialized bit in the event group
    xEventGroupSetBits(motor_driver_event_group, MOTOR_DRIVER_INITIALIZED_BIT);
    ESP_LOGI(TAG, "Motor driver initialized.");
    return ESP_OK;
}

void motor_driver_set_state(motor_driver_state_t state)
{
    motor_driver_state = state;
}

void motor_driver_set_voltage_helper(float voltage, gpio_num_t in1, gpio_num_t in2, ledc_channel_t pwm_channel, bool free_spin)
{
    uint32_t duty = 0;
    // Set motor voltage
    if (motor_driver_state == MOTORS_DISABLED || free_spin)
    {
        // Set both IN1 and IN2 low to disable the motor (free spin)
        gpio_set_level(in1, 0);
        gpio_set_level(in2, 0);
        duty = 0;
    }
    else if (motor_driver_state == MOTORS_SHORT_BRAKE)
    {
        // If in SHORT_BRAKE state, set both IN1 and IN2 high to brake the motor
        gpio_set_level(in1, 1);
        gpio_set_level(in2, 1);
        duty = 0;
    }
    else // MOTORS_ENABLED
    {
        // Set motor direction based on voltage sign
        if (voltage >= 0.0f)
        {
            gpio_set_level(in1, 0);
            gpio_set_level(in2, 1);
        }
        else
        {
            gpio_set_level(in1, 1);
            gpio_set_level(in2, 0);
        }
        // Calculate duty cycle based on bus voltage and desired voltage
        duty = bus_voltage >= cfg.min_bus_voltage ? (uint32_t)(fabsf(voltage / bus_voltage * (float)(1 << MOTOR_PWM_RESOLUTION))) : 0;
        // Clamp duty to the maximum value to prevent overflow
        duty = duty < (1 << MOTOR_PWM_RESOLUTION) ? duty : (1 << MOTOR_PWM_RESOLUTION) - 1;
    }

    // Set PWM duty cycle for the specified motor
    ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, pwm_channel);
}

void motor_driver_set_voltage(const float voltage[2])
{
    motor_driver_set_voltage_helper(voltage[0], MOTOR_A_IN1, MOTOR_A_IN2, LEDC_CHANNEL_0, cfg.motor_A_free_spin);
    motor_driver_set_voltage_helper(voltage[1], MOTOR_B_IN1, MOTOR_B_IN2, LEDC_CHANNEL_1, cfg.motor_B_free_spin);
}

void motor_driver_set_voltage_motor_A(float voltage)
{
    motor_driver_set_voltage_helper(voltage, MOTOR_A_IN1, MOTOR_A_IN2, LEDC_CHANNEL_0, cfg.motor_A_free_spin);
}

void motor_driver_set_voltage_motor_B(float voltage)
{
    motor_driver_set_voltage_helper(voltage, MOTOR_B_IN1, MOTOR_B_IN2, LEDC_CHANNEL_1, cfg.motor_B_free_spin);
}

void update_bus_voltage_task(void *arg)
{
    // Register task with the safety system
    ESP_ERROR_CHECK(safety_system_register_task());

    // Wait for the motor driver to be initialized
    xEventGroupWaitBits(motor_driver_event_group, MOTOR_DRIVER_INITIALIZED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Main loop to periodically update bus voltage
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (!safety_system_is_shutting_down())
    {
        // Update bus voltage
        motor_driver_update_bus_voltage();
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(cfg.bus_voltage_update_interval_ms));
    }

    // Cleanup and exit the task
    ESP_ERROR_CHECK(safety_system_unregister_task());
    ESP_LOGI(TAG, "Update bus voltage task finished.");
    vTaskDelete(NULL);
}
