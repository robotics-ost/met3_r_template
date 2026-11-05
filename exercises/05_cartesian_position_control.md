# Exercise 05: Position Control in Cartesian Space

## Learning objectives

You can implement a position controller in Cartesian space for a mobile robot with differential drive.

## Task 5: Position control in Cartesian space

1. Draw the block diagram of the complete system. Combine functional components, such as the inverse kinematics, into a single block.
2. Implement the position controller for differential-drive robots based on the provided starter code (`position_controller_diff_drive`) and integrate it into the control system.
3. Implement a new step in the sequencer that sets a new target pose.
   - Adapt the web interface accordingly.
4. Test your implementation by sending different target poses to the robot via the web interface.
   - Test different values for the parameters K1, K2 and K3.
   - How accurately can the robot return to its starting point? What influences the accuracy?
   - How does the robot react to a change of the safety level?

## Bonus task

Extend the algorithm so that the robot corrects an error in its orientation, even if it has already reached the target position.

## Starter code

Copy the following files from `exercises/starter_code/` into the control system library. The places where you have to add your own code are marked with `/* TO DO */`. What each function must do is described in the header file (`.h`).

| File | Copy to |
|---|---|
| `position_controller_diff_drive.h` | `lib/control_system/include/` |
| `position_controller_diff_drive.c` | `lib/control_system/src/` |

You can copy the files in the VS Code Explorer (right-click &rarr; **Copy**, then right-click on the target folder &rarr; **Paste**) or with the following commands in the VS Code terminal:

```bash
cp exercises/starter_code/position_controller_diff_drive.h lib/control_system/include/
cp exercises/starter_code/position_controller_diff_drive.c lib/control_system/src/
```

> [!IMPORTANT]
> Copy the files, don't move them. Never edit the files in `exercises/starter_code/` directly.

How to integrate a new module into the control system is explained step by step in [Working with the Code](../docs/WORKING_WITH_THE_CODE.md).

## Hints

- How to add a new step type on the PC side is described in the [host interface guide](../host_interface/README.md), section *B. Changing sequencer step types*.

## Done when

- [ ] the robot drives to a target pose that you send from the web GUI,
- [ ] you can explain the influence of K1, K2 and K3,
- [ ] you can explain how the robot reacts to a change of the safety level.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-05`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
