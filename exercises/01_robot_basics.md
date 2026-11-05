# Exercise 01: Robot Basics, Encoders and Motor Voltage

## Learning objectives

You can:

- describe the hardware and the software architecture of the robot,
- read the encoder positions of the robot,
- set the motor voltages of the robot.

## Task 1.1: Set up the development environment

1. Install Git, VS Code, PlatformIO and Python as described in [Getting Started](../docs/GETTING_STARTED.md).
2. Create your own repository from the starter code ([Quick Start](../README.md#quick-start), Steps 2&ndash;4).
3. Get an overview of the repository: read the sections *Project structure* and *Architecture Overview* in the [main README](../README.md#project-structure).

## Task 1.2: Connect the robot to WiFi

1. Connect the robot to the WiFi network ([Quick Start](../README.md#step-5-enter-the-wifi-credentials), Step 5). Your lecturer tells you the network name (SSID) and the password. Alternatively, you can use a hotspot on your laptop (**2.4 GHz only**, see the tip in Step 5).
2. Choose a unique hostname for your robot. Agree on the names with your fellow students ([Quick Start](../README.md#step-6-give-your-robot-a-unique-name), Step 6).
3. Check that you can connect to the robot with the web GUI ([Quick Start](../README.md#step-8-connect-to-the-robot-from-your-pc), Step 8).

## Task 1.3: Read the encoders

1. Extend the control system so that the encoder positions are read every 100 ms and sent to the host PC via UDP.
2. Check in the web GUI that the data is received.

## Task 1.4: Set the motor voltage

1. Extend the control system so that, in addition, the motor voltages are set in every control cycle as follows:

   ```text
   U_L = sin(0.2 * pi * t)
   U_R = 2 * triangle(0.5 * t) - 1
   ```

2. Change the safety level in the web GUI and observe how the robot reacts.

## Starter code

This exercise needs no additional starter code. You mainly work in `lib/control_system/src/control_system.c`.

## Hints

- [Working with the Code](../docs/WORKING_WITH_THE_CODE.md) explains how the control loop works and where to put your code.
- Useful functions: `encoder_driver_get_angles()`, `motor_driver_set_voltage()` and `udp_interface_add_control_sample()`. Click on the function names in the [online documentation](https://robotics-ost.github.io/met3_r_template/) to see how they are used.
- The period of the control loop is set in `include/config.h` (`CONTROL_SYSTEM_TASK_PERIOD_MS`, default 5 ms).
- There is no `triangle()` function in C. Write it yourself.

## Done when

- [ ] the web GUI shows the encoder angles changing when you turn a wheel by hand,
- [ ] the wheels move as described in Task 1.4,
- [ ] you can explain the purpose of the components Safety System, Control System, Sequencer and HAL, and how they interact,
- [ ] you can explain how the robot reacts to each safety level.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-01`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
