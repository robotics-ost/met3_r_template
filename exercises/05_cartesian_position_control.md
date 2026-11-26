# Exercise 05: Position Control in Cartesian Space

## Learning objectives

You can implement a position controller in Cartesian space for a mobile robot with differential drive.

You can explain how a command from the PC reaches the control system via the sequencer.

## Task 5: Position control in Cartesian space

1. Draw the block diagram of the complete system. Combine functional components, such as the inverse kinematics, into a single block.
2. Implement the position controller for differential-drive robots based on the provided starter code (`position_controller_diff_drive`) and integrate it into the control system.
3. Connect the `MOVE_TO` step of the sequencer to the position controller. The step itself and the **Move to** form in the web GUI are already provided, you only have to add the calls to the position controller. The places are marked with `/* TO DO */` in `lib/sequencer/src/sequencer.c`.
4. Test your implementation by sending different target poses to the robot via the **Move to** form of the web GUI.
   - Test different values for the parameters K1, K2 and K3.
   - How accurately can the robot return to its starting point? What influences the accuracy?
   - What happens if you remove the active `MOVE_TO` step from the queue while the robot is driving?
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

- The sequencer and the control system run in different tasks. Which functions of the position controller are called by the sequencer, and which one by the control system? The provided functions show how the two tasks share data safely. Read them before you implement `position_controller_run()`.
- How the sequencer works and how its steps are executed is explained in [Sequencer](../docs/SEQUENCER.md). The big picture of all components is in [Architecture Concept](../docs/SYSTEM_ARCHITECTURE.md) and [Robot Software](../docs/ROBOT_SOFTWARE.md).

## Done when

- [ ] the robot drives to a target pose that you send from the web GUI,
- [ ] the robot stops when you remove the active `MOVE_TO` step from the queue,
- [ ] you can explain the influence of K1, K2 and K3,
- [ ] you can explain how a target pose gets from the web GUI to the position controller,
- [ ] you can explain how the robot reacts to a change of the safety level.

## Save your solution

When you are done, save your solution on GitHub and mark it with a tag, so you can come back to it later:

1. **Stage, commit and push:** click the **Source Control** icon in the left side bar, stage your changes (**+** icon next to each file), enter a short message, click **Commit**, then **Sync Changes**.
2. **Create a tag:** open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag** and enter `exercise-05`.
3. **Push the tag:** open the **Command Palette** again and run **Git: Push Tags**.

Details and help with problems: [Git Guide](../docs/GIT_GUIDE.md).
