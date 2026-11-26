# UDP Interface

## The concept

The communication interface connects the robot to the outside, e.g. to a PC. It transports data in two directions:

- **Telemetry** goes from the robot to the PC. It is a continuous stream of data for monitoring, e.g. for plots.
- **Commands** go from the PC to the robot, e.g. a safety event or a new step for the sequencer.

The interface only **transports** data. It decides nothing: commands are passed on to the component that is responsible (safety system or sequencer), and telemetry is collected from the other components.

It is *not* real-time and runs with a low priority. The communication must never disturb the control loop. That's why the other components don't send their data directly. They put their samples into a queue, which doesn't block, and the communication task sends the samples later. If the queue is full, samples are dropped instead of slowing down the sender.

To save network traffic, several samples are sent together in one packet (a *batch*).

See also [Architecture Concept](SYSTEM_ARCHITECTURE.md).

## In the robot software

### Connection

- The robot connects to the WiFi network as a client. The network name and the password are in `include/wifi_secrets.h`.
- The robot is reachable by its hostname (e.g. `robot_1.local`, set with `UDP_INTERFACE_HOSTNAME` in `include/config.h`). It prints its IP address in the Serial Monitor (`Got IP: ...`).
- The data is sent with UDP on the port `UDP_INTERFACE_UDP_PORT`. UDP has no connection and no confirmation: a lost packet is not sent again. For telemetry this is no problem, a missing sample in a plot doesn't matter. The same holds for commands: the robot doesn't confirm them.

### Telemetry

As soon as a PC is registered, the UDP task sends one packet in every cycle (`UDP_INTERFACE_TASK_PERIOD_MS`). A packet contains several samples of

- the **safety level** (added by the safety system with `udp_interface_add_safety_sample()`),
- the **control values** that are plotted in the web GUI (added by the control system with `udp_interface_add_control_sample()`),
- the first steps of the **sequencer queue**.

If a queue is full, the Serial Monitor shows the warning that samples were dropped.

### Commands

The UDP interface understands the following commands from the PC:

- register and unregister the PC (the first packet of the web GUI),
- trigger a safety event,
- add a step to the sequencer queue (*wait*, *move to*),
- remove a step from the sequencer queue.

Only **one PC at a time** is registered. A new registration replaces the old one. Commands from other computers are ignored.

### Adapting the interface

The protocol, and the places that you have to change when you send other values or add commands, are described in the [host interface guide](../host_interface/README.md). How to use the PC side is explained in [Web GUI](WEB_GUI.md).
