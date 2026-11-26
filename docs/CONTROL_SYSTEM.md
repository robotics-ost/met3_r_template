# Control System

## The concept

The control system is the **fast, periodic control loop** of the robot. In every cycle it

1. reads the sensors,
2. calculates (e.g. kinematics and controllers),
3. sets the actuators,
4. and queues the data to be sent to the outside, e.g. for plotting.

It is *real-time*: every cycle must be finished within its period, every time. That's why nothing inside the loop may wait or block, and why it has a high priority.

The control system is built from small **modules**, for example a motor model, controllers or the kinematics. Each module does one calculation. The control loop connects them: the output of one module is the input of the next. Other components, e.g. the sequencer, only use the functions of a module.

See also [Architecture Concept](SYSTEM_ARCHITECTURE.md).

### Sharing data with other tasks

The control loop runs in its own task. As soon as another task, e.g. the sequencer, calls a function of a control system module, two tasks use the same data at the same time. Such data must be protected:

- **Data that consists of several values** (e.g. a target position) is protected by a *mutex*. A task takes the mutex before it reads or writes the data, and gives it back afterwards.
- **Simple states** (e.g. "target reached") can be stored as bits in an *event group*.

Two rules for code that runs in the control loop:

- Keep the time in which you hold a mutex as short as possible. Copy the shared data into local variables, give the mutex back and calculate with the copies.
- Never wait for something else (e.g. a function that can block) while you hold a mutex. The control loop must keep its period.

## In the robot software

### The control task

The function `control_system_task()` in `lib/control_system/src/control_system.c` is one task with an endless loop. It runs periodically (`CONTROL_SYSTEM_TASK_PERIOD_MS` in `include/config.h`) and has the four parts described above. [Working with the Code](WORKING_WITH_THE_CODE.md) shows the loop and explains how to add your own modules to it.

> [!WARNING]
> Never wait inside the control loop (no `vTaskDelay()`, no endless loops, no long computations).

### Modules

Every module has

- a config struct and an `..._init()` function that is called once in `control_system_init()`,
- one or more functions that you call in the control loop.

**You** connect the modules in the control loop.

### Example: the encoder driver

The encoder driver of the [HAL](HAL.md) shows the typical pattern of a module:

- **Config:** `encoder_driver_config_t` holds the pulses per revolution of the encoders. The values are defined in `include/config.h` and filled into the control system config in `app_main()` in `src/main.c`.
- **Init:** `encoder_driver_init()` is called once in `control_system_init()`.
- **Use:** in Exercise 1 you call `encoder_driver_get_angles()` in the control loop to read the shaft angles.

### The control system and the safety level

The control system doesn't depend on the safety level: it keeps calculating when the motors are disabled. See [Safety System](SAFETY_SYSTEM.md#what-the-safety-level-does-not-do).
