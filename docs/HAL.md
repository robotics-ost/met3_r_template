# Hardware Abstraction Layer (HAL)

## The concept

The HAL provides a simple interface to the other components of the robot software, so that they do not have to deal with the hardware details.

Instead of pins, registers and raw counter values, the other components use functions with physical units, e.g. "set the voltage of motor A" or "get the shaft angle in radians". Only the HAL knows which chips, pins and peripherals are used.

This has several advantages:

- **Exchangeable hardware:** if a motor driver or an encoder is replaced, only the HAL changes. The control system, the sequencer and the safety system stay the same.
- **Simulation and testing:** the HAL can be replaced by a simulation, so that the rest of the software can run without the robot.
- **One place for hardware knowledge:** all other components stay simple and easy to read.

Only the HAL touches the hardware. Everything else uses the functions of the HAL.

See also [Architecture Concept](SYSTEM_ARCHITECTURE.md).

## In the robot software

The HAL has no task of its own: its functions are called by the other tasks. Only the bus voltage of the motors is updated by a small background task.

The HAL controls the physical hardware:

- **Motor driver** (`lib/hal/include/motor_driver.h`): TB6612FNG via PWM. The voltage is set per motor, including the direction. The bus voltage is monitored with an INA219.
- **Encoder driver** (`lib/hal/include/encoder_driver.h`): quadrature decoding with the ESP-IDF PCNT peripheral. Two encoders with configurable pulses per revolution.

### Motor driver

- `motor_driver_set_voltage()` sets the voltage of both motors (positive: forward, negative: reverse).
- Whether the motors turn at all is decided by the safety system: it enables and disables the motors with `motor_driver_set_state()`. Only the safety system calls this function. The motors are only enabled in the safety level *Active* (see [Safety System](SAFETY_SYSTEM.md)).
- The bus voltage is read periodically by a background task (`update_bus_voltage_task()`). If it is lower than the minimum (`MOTOR_DRIVER_MIN_BUS_VOLTAGE` in `include/config.h`), the motors don't turn and the Serial Monitor shows a warning.

### Encoder driver

- `encoder_driver_get_angles()` returns the angles of both encoders in radians.
- The angles are the angles of the input shaft of the motor. The transmission ratio is not taken into account.
- The pulses per revolution of the encoders are set in `include/config.h` (`ENCODER_DRIVER_ENC_A_PPR` and `ENCODER_DRIVER_ENC_B_PPR`).

### Initialization

Both drivers are initialized once in `control_system_init()`. Their configuration is filled in `app_main()` in `src/main.c` from the constants in `include/config.h`.
