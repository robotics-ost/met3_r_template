# Exercise 04: Kinematics of a Differential-Drive Robot

## Learning objectives

You can implement the kinematics (forward kinematics and odometry, inverse kinematics) of a mobile robot with differential drive.

## Task 4.1: Forward kinematics and odometry

1. Implement the equations for the forward kinematics and odometry of a differential-drive robot based on the provided starter code (`kinematics`).
2. Integrate your implementation into the control system:
   *read encoders &rarr; motor model &rarr; forward kinematics and odometry*
3. Send the pose and velocity of the robot to your PC via UDP.
   - Adapt the web interface so that the values are displayed correctly.
4. Test your implementation:
   - Move the robot by hand and observe the displayed values.
   - Are the values correct for a pure rotation?
   - Are the values correct for a pure translation?

## Task 4.2: Inverse kinematics

1. Implement the equations for the inverse kinematics of a differential-drive robot based on the provided starter code.
2. Integrate your implementation into the control system:
   *inverse kinematics &rarr; PI controller &rarr; motor model &rarr; set motor voltage*
3. Test your implementation:
   - Set a pure translational velocity. Does the robot really drive straight ahead?
   - Set a pure angular velocity. Does the robot really turn on the spot?

## Starter code

Copy the following files from `exercises/starter_code/` into the control system library. The places where you have to add your own code are marked with `/* TO DO */`. What each function must do is described in the header file (`.h`).

| File | Copy to |
|---|---|
| `kinematics.h` | `lib/control_system/include/` |
| `kinematics.c` | `lib/control_system/src/` |

You can copy the files in the VS Code Explorer (right-click &rarr; **Copy**, then right-click on the target folder &rarr; **Paste**) or with the following commands in the VS Code terminal:

```bash
cp exercises/starter_code/kinematics.h lib/control_system/include/
cp exercises/starter_code/kinematics.c lib/control_system/src/
```

> [!IMPORTANT]
> Copy the files, don't move them. Never edit the files in `exercises/starter_code/` directly.

How to integrate a new module into the control system is explained step by step in [Working with the Code](../docs/WORKING_WITH_THE_CODE.md).

## Hints

- To send other values than the encoder positions, you have to change the telemetry on both sides: in `lib/udp_interface/` (ESP32) and in `host_interface/` (PC). The [host interface guide](../host_interface/README.md) lists all places to change in section *A. Changing what's in a control sample*.
- In which frame are the velocities given? Check the documentation of `kinematics_calc_fw_kin_odom()` and `kinematics_calc_inv_kin()`.

## Done when

- [ ] the displayed pose is correct for a pure rotation and a pure translation,
- [ ] the robot drives straight ahead for a pure translational velocity,
- [ ] the robot turns on the spot for a pure angular velocity.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-04`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
