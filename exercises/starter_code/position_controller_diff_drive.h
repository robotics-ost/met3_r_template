/**
 * @file position_controller_diff_drive.h
 * @brief Cartesian position controller for differential drive robots header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>

/**
 * @brief Configuration structure for the position controller.
 */
typedef struct
{
    float k1;                    /**< Position control parameter 1 [1/s] */
    float k2;                    /**< Position control parameter 2 [1/s] */
    float k3;                    /**< Position control parameter 3 [-] */
    float position_tolerance;    /**< Position tolerance to consider target reached [m] */
    float orientation_tolerance; /**< Orientation tolerance to consider target reached [rad] */
    float max_linear_velocity_x; /**< Maximum allowed linear robot velocity in x direction [m/s] */
    float max_angular_velocity;  /**< Maximum allowed angular robot velocity [rad/s] */
} position_controller_config_t;

/**
 * @brief Initializes the position controller with the given configuration.
 *
 * @param config Pointer to the position controller configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t position_controller_init(const position_controller_config_t *config);

/**
 * @brief Sets the target position and orientation for the position controller.
 *
 * @param xi_d Pointer to an array containing the desired position and orientation [x, y, phi].
 *
 * @return true if the target was successfully set, false if the input is invalid (e.g., NULL pointer).
 */
bool position_controller_set_target(const float xi_d[3]);

/**
 * @brief Checks if the target position and orientation has been reached.
 *
 * @return true if the target is reached, false otherwise.
 */
bool position_controller_is_target_reached(void);

/**
 * @brief Aborts the current position control operation.
 *
 * This function is called by the sequencer task. It only requests the abort: the robot is stopped by
 * position_controller_run() in its next call.
 */
void position_controller_abort(void);

/**
 * @brief Runs the position controller.
 *
 * Call this function in every cycle of the control loop. It calculates the desired robot velocities that
 * drive the robot to the target pose.
 *
 * The functions position_controller_set_target() and position_controller_abort() are called by the sequencer task,
 * this function by the control task. They share the target pose (protected by the mutex `_xi_d_mutex`) and the
 * states "target reached" and "target aborted" (event group bits `TARGET_REACHED_BIT` and `TARGET_ABORTED_BIT`).
 * In every call, this function has to:
 * - Read the target pose into a local variable. Hold the mutex only as long as needed for copying.
 * - Check if an abort was requested (`TARGET_ABORTED_BIT`). If yes: clear the bit, stop the robot (all desired
 *   velocities 0) and set `TARGET_REACHED_BIT`, so that the controller is idle as after position_controller_init().
 * - Output zero velocities if there is nothing to do (`TARGET_REACHED_BIT` is set).
 * - Otherwise calculate the desired robot velocities and limit them to the maximum velocities of the configuration.
 * - Check if the target is reached (`position_tolerance`, and `orientation_tolerance` if the orientation is
 *   controlled as well). If yes: set `TARGET_REACHED_BIT` and output zero velocities.
 *
 * The function must not block: it runs in the control loop.
 *
 * @param xi Pointer to an array containing the current position and orientation in the global frame [x, y, phi] [m, m, rad].
 * @param xid_d Pointer to an array where the desired robot velocities will be stored in the robot frame [vx, vy, omega] [m/s, m/s, rad/s].
 */
void position_controller_run(const float xi[3], float xid_d[3]);
