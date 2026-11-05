# Exercise 06: End-Effector (TCP) Control

## Learning objectives

You can:

- implement the kinematics of the end effector (TCP) of a mobile robot with differential drive,
- implement a TCP position controller for a mobile robot with differential drive.

## Task 6.1: Kinematics of the end effector

1. Implement the kinematics of the end effector.
   - Create your own `.h` and `.c` files.
   - Think about which values you will get from the position controller that you implement in the next task. Draw the complete control system as a block diagram.
2. Use your previous implementation of the kinematics as starting point.
3. Integrate the kinematics into the control system and test your implementation.

## Task 6.2: End-effector position control

1. Implement the end-effector position controller.
   - Create your own `.h` and `.c` files.
2. Use your implementation of the position controller for differential-drive robots as a guide.
3. Integrate the end-effector position controller into the control system and test your implementation.

## Starter code

There is no starter code for this exercise.

## Hints

- To create your own module, copy the structure of an existing one, e.g. `kinematics.h` and `kinematics.c`: a config struct, an init function and the calculation functions. Use a new prefix for all names (e.g. `tcp_kinematics_...`), otherwise the names clash with the existing module.
- Put the `.h` file into `lib/control_system/include/` and the `.c` file into `lib/control_system/src/`. PlatformIO finds them automatically.

## Done when

- [ ] the robot moves its end effector (TCP) to a target position that you send from the web GUI.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-06`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
