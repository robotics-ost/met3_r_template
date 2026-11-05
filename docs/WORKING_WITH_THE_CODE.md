# Working with the Code

This guide explains where your code goes, how to add a new module (e.g. a controller) and how to see what your code is doing.

> [!IMPORTANT]
> **Most of your code goes into the control loop in `lib/control_system/src/control_system.c`.**
> Besides that, you change `include/config.h` and `src/main.c` for the configuration. In later exercises, you also extend other components, e.g. the telemetry or the sequencer.

## How the program runs

1. `app_main()` in `src/main.c` runs once at start-up. It initializes all components and then starts their tasks.
2. Each component runs as its own **task**, a function with an endless loop that runs periodically (see the table in the [Architecture Overview](../README.md#architecture-overview)).
3. The **control system task** `control_system_task()` runs every `CONTROL_SYSTEM_TASK_PERIOD_MS` (default 5 ms, set in `include/config.h`). This is where you read the sensors, calculate and set the actuators:

```c
while (!safety_system_is_shutting_down())
{
    // 1. Read the sensors        e.g. encoder_driver_get_angles(...)
    // 2. Calculate               e.g. your motor model, controllers, kinematics
    // 3. Set the actuators       e.g. motor_driver_set_voltage(...)
    // 4. Send telemetry          udp_interface_add_control_sample(...)

    // Wait for the next cycle
    xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.task_period_ms));
}
```

> [!WARNING]
> Never use `vTaskDelay()`, busy loops or anything else that waits inside the control loop. Each cycle must be finished well within the control period.

## Adding a module from the starter code

All exercises from Exercise 02 on follow the same five steps. The example uses the motor model (`motor_model`), the other modules work the same way.

### Step 1: Copy the files

Copy `motor_model.h` to `lib/control_system/include/` and `motor_model.c` to `lib/control_system/src/`. PlatformIO finds them automatically.

### Step 2: Fill in the TO DOs

Search for `/* TO DO */` in the copied `.c` file (`Ctrl+F`) and implement the missing code. What each function must do, and what its parameters and config struct mean, is described in the comments in the header file (`.h`).

### Step 3: Add the configuration

All constants belong into `include/config.h` (see [Configuration](../README.md#configuration)). This takes three small changes:

**a) `include/config.h`:** define the constants.

```c
// Motor model configuration constants
#define MOTOR_MODEL_ARMATURE_RESISTANCE 10.0f /**< Armature resistance [Ohm], take the value from the data sheet */
// ...
```

**b) `lib/control_system/include/control_system.h`:** add the config struct of the module to the control system config. There is already a comment marking the place.

```c
#include "motor_model.h"

typedef struct
{
    uint32_t task_period_ms;
    encoder_driver_config_t encoder_config;
    motor_driver_config_t motor_driver_config;
    motor_model_config_t motor_model_config; /**< Configuration for the motor model */
} control_system_config_t;
```

**c) `src/main.c`:** fill in the new struct in `app_main()`.

```c
static const motor_model_params_t motor_params[2] = {
    {.armature_resistance = MOTOR_MODEL_ARMATURE_RESISTANCE, /* ... */},
    {.armature_resistance = MOTOR_MODEL_ARMATURE_RESISTANCE, /* ... */},
};

const control_system_config_t control_config = {
    // ... existing fields ...
    .motor_model_config = {
        .dt = CONTROL_SYSTEM_TASK_PERIOD_S,
        .params = motor_params,
        .nr_of_motors = 2,
    },
};
```

> [!CAUTION]
> Some config structs only store a **pointer** to an array (e.g. `params` or the mass matrix `M`). The array must still exist when the init function has returned, so declare it with **`static`**, as shown above. Without `static`, the array is deleted at the end of `app_main()` and your module silently reads garbage.

### Step 4: Initialize the module

In `lib/control_system/src/control_system.c`, call the init function in `control_system_init()`, next to the existing ones:

```c
ESP_ERROR_CHECK(motor_model_init(&cfg.motor_model_config));
```

You don't need an extra `#include` here: `control_system.c` includes `control_system.h`, which already includes `motor_model.h` since Step 3b.

`ESP_ERROR_CHECK()` stops the program with an error message in the Serial Monitor if the init function fails, e.g. because a parameter is invalid.

### Step 5: Use the module in the control loop

Call the functions of the module in the control loop, e.g. `motor_model_calc_q(shaft_angles, q);`.

## Seeing what your code does

### Messages in the Serial Monitor

Use `ESP_LOGI()` instead of `printf()`. The messages appear in the Serial Monitor:

```c
ESP_LOGI(TAG, "q = [%.3f, %.3f] rad", q[0], q[1]);
```

> [!WARNING]
> Do not print a message in every cycle of the control loop (with the default period, that's 200 times per second). This floods the Serial Monitor and slows the robot down.

Print only every 100th cycle instead, with a counter:

```c
uint32_t cycle_count = 0; // before the while loop

while (!safety_system_is_shutting_down())
{
    // ...

    if (cycle_count % 100 == 0) // true in every 100th cycle
    {
        ESP_LOGI(TAG, "q = [%.3f, %.3f] rad", q[0], q[1]);
    }
    cycle_count++;

    xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.task_period_ms));
}
```

The same counter trick works for everything that should happen less often than every cycle.

### Plots in the web GUI

Values sent with `udp_interface_add_control_sample()` are plotted in the web GUI. Currently, two values are sent (the two encoder positions). To send other or more values, follow section *A. Changing what's in a control sample* in the [host interface guide](../host_interface/README.md).

## Vectors and arrays

Most functions work on several motors at once. They take an array plus its length `dim` (or `nr_of_motors`) and return their result through a pointer parameter instead of a return value:

```c
float q[2];                          // input: positions of both motors
float Q[2];                          // output: filled in by the function
pd_controller_calc_Q(q, q_d, Q);     // Q now contains the result
```

Matrices are stored row by row in a flat array: the element in row `i`, column `j` of an `n x n` matrix is `M[i * n + j]`.

New to C? Pointers, arrays, structs and `static` are explained in [C for Python Programmers](C_FOR_PYTHON_PROGRAMMERS.md).
