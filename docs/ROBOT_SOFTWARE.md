# Robot Software

This page shows how the [architecture concept](SYSTEM_ARCHITECTURE.md) is built in the software of the robot.

## Components and folders

| Component | Folder | Page |
|---|---|---|
| **Safety System** | `lib/safety_system/` | [Safety System](SAFETY_SYSTEM.md) |
| **Control System** | `lib/control_system/` | [Control System](CONTROL_SYSTEM.md) |
| **Sequencer** | `lib/sequencer/` | [Sequencer](SEQUENCER.md) |
| **Hardware Abstraction Layer (HAL)** | `lib/hal/` | [HAL](HAL.md) |
| **UDP Interface** (communication with your PC) | `lib/udp_interface/` | [UDP Interface](UDP_INTERFACE.md) |

## Tasks, cores and periods

Each component runs as its own task. They are created in `src/main.c`. The periods are set in `include/config.h`.

| Task | Core | Priority | Period set by |
|---|---|---|---|
| Safety System | 1 | 5 | `SAFETY_SYSTEM_TASK_PERIOD_MS` |
| Control System | 1 | 4 | `CONTROL_SYSTEM_TASK_PERIOD_MS` |
| UDP Interface | 0 | 3 | `UDP_INTERFACE_TASK_PERIOD_MS` |
| Sequencer | 0 | 3 | `SEQUENCER_TASK_PERIOD_MS` |

Core 1 runs the real-time tasks (safety and control), core 0 runs the others. A higher number means a higher priority.

The HAL has no task of its own: its functions are called by the other tasks.
