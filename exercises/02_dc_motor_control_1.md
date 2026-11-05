# Exercise 02: DC Motor Control, Part 1

## Learning objectives

You can:

- implement a model of a DC motor,
- implement a servo controller.

## Task 2.1: DC motor model

1. Implement the model of a DC motor based on the provided starter code (`motor_model`).
2. Integrate the model into the control system:
   - With a positive voltage, the wheel must turn so that the robot drives in the positive direction.
   - If the robot is pushed in the positive direction, the position `q` must increase.
3. Test the model:
   - Set different values for the torque `Q` and the velocity `qd` (time derivative of `q`).
   - Turn the motor by hand and check whether the position `q` is calculated correctly.

## Task 2.2: Position controller

1. Draw the block diagram and calculate the parameters of the PD controller.
2. Implement the PD controller based on the provided starter code (`pd_controller`).
3. Test the controller by using the position `q` of one motor as the setpoint for the other motor.
   - Use the DC motor model to calculate the voltage from the torque given by the controller and the current velocity.

## Starter code

Copy the following files from `exercises/starter_code/` into the control system library. The places where you have to add your own code are marked with `/* TO DO */`. What each function must do is described in the header file (`.h`).

| File | Copy to |
|---|---|
| `motor_model.h` | `lib/control_system/include/` |
| `motor_model.c` | `lib/control_system/src/` |
| `pd_controller.h` | `lib/control_system/include/` |
| `pd_controller.c` | `lib/control_system/src/` |

You can copy the files in the VS Code Explorer (right-click &rarr; **Copy**, then right-click on the target folder &rarr; **Paste**) or with the following commands in the VS Code terminal:

```bash
cp exercises/starter_code/motor_model.h lib/control_system/include/
cp exercises/starter_code/motor_model.c lib/control_system/src/
cp exercises/starter_code/pd_controller.h lib/control_system/include/
cp exercises/starter_code/pd_controller.c lib/control_system/src/
```

> [!IMPORTANT]
> Copy the files, don't move them. Never edit the files in `exercises/starter_code/` directly.

How to integrate a new module into the control system is explained step by step in [Working with the Code](../docs/WORKING_WITH_THE_CODE.md).

## Hints

- Check the period of the control loop in `include/config.h` (`CONTROL_SYSTEM_TASK_PERIOD_MS`). Which value makes sense for a controller?
- The direction of rotation can be inverted with the parameter `invert_direction`, without changing your equations.
- Take the motor parameters from the motor data sheet and convert them to SI units.
- `motor_model_config_t` and `pd_controller_config_t` only store a **pointer** to the parameters (`params`, `M`). The data the pointer points to must still exist after the init function has returned, so declare it as `static const` (see [Working with the Code](../docs/WORKING_WITH_THE_CODE.md)).

## Done when

- [ ] the position `q` increases when you push the robot in the positive direction,
- [ ] a positive voltage drives the robot in the positive direction,
- [ ] the controlled motor follows the motor you turn by hand.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-02`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
