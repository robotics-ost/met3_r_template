# Exercise 03: DC Motor Control, Part 2

## Learning objectives

You can:

- implement a steer-by-wire algorithm,
- implement a PI velocity controller.

## Task 3.1: Steer-by-wire

Use the PD position controller from the last exercise to control both motors at the same time. Each motor uses the position of the **other** motor as its setpoint.

1. Extend the block diagram of the controller. Tip: use vectors at the inputs and outputs of the controller.
2. Implement and test your algorithm.

## Task 3.2: Velocity controller

1. Draw the block diagram and calculate the parameters of the PI controller.
2. Implement the PI controller based on the provided starter code (`pi_controller`).
   - The maximum velocity of the motor should be reached after one revolution of the setpoint wheel.
   - Think about in which [safety levels](../lib/safety_system/include/safety_system.h) (`safety_level_t`) you should enable the integrator, in which you should disable it, and when you should reset it.
3. Test the controller by using the position `q` of one motor as the setpoint for the velocity `qd` (time derivative of `q`) of the other motor.
   - Use the DC motor model to calculate the voltage from the torque given by the controller and the current velocity.

## Starter code

Copy the following files from `exercises/starter_code/` into the control system library. The places where you have to add your own code are marked with `/* TO DO */`. What each function must do is described in the header file (`.h`).

| File | Copy to |
|---|---|
| `pi_controller.h` | `lib/control_system/include/` |
| `pi_controller.c` | `lib/control_system/src/` |

You can copy the files in the VS Code Explorer (right-click &rarr; **Copy**, then right-click on the target folder &rarr; **Paste**) or with the following commands in the VS Code terminal:

```bash
cp exercises/starter_code/pi_controller.h lib/control_system/include/
cp exercises/starter_code/pi_controller.c lib/control_system/src/
```

> [!IMPORTANT]
> Copy the files, don't move them. Never edit the files in `exercises/starter_code/` directly.

How to integrate a new module into the control system is explained step by step in [Working with the Code](../docs/WORKING_WITH_THE_CODE.md).

## Hints

- With `dim = 2`, the controllers work on vectors. Index 0 and 1 stand for the two motors.
- The integrator is controlled with `pi_controller_enable_integrator()`, `pi_controller_disable_integrator()` and `pi_controller_reset_integrator()`.
- `safety_system_get_level()` returns the current safety level. All safety levels are listed in `safety_level_t` in `lib/safety_system/include/safety_system.h`.

## Done when

- [ ] turning one wheel by hand makes the other wheel follow, in both directions (steer-by-wire),
- [ ] the velocity of the controlled motor follows the position of the setpoint wheel,
- [ ] you can explain in which safety levels the integrator is enabled, disabled and reset, and why.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-03`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
