# Templates

This directory contains the template code used for the various MeT3_R practical exercises. You can copy and paste the commands listed below under the corresponding exercise section to copy the template files to the location where they are needed. Just make sure that you are in the main project directory (i.e. the parent directory of the directory containing this README). Alternatively, copy the files manually.

Each template marks the sections where you have to add your own code with `/* TO DO */` comment blocks.

## Reading the encoder data and setting the motor voltage

These exercises do not need any additional template code.

## DC-Motor Model

```bash
cp templates/motor_model.h lib/control_system/include
cp templates/motor_model.c lib/control_system/src
```

## PD-Position Control

```bash
cp templates/pd_controller.h lib/control_system/include
cp templates/pd_controller.c lib/control_system/src
```

## Steer-by-Wire

This exercise builds on the previous exercises and does not require any additional template code.

## PI-Velocity Control

```bash
cp templates/pi_controller.h lib/control_system/include
cp templates/pi_controller.c lib/control_system/src
```

## Kinematics

```bash
cp templates/kinematics.h lib/control_system/include
cp templates/kinematics.c lib/control_system/src
```

## Position Control in the Cartesian Space

```bash
cp templates/position_controller_diff_drive.h lib/control_system/include
cp templates/position_controller_diff_drive.c lib/control_system/src
```

## TCP Control


