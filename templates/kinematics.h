/**
 * @file kinematics.h
 * @brief Kinematics header file for the robot system.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <esp_err.h>

/**
 * @brief Kinematics configuration structure
 */
typedef struct
{
    float dt;             /**< Control system period [s] */
    float wheel_distance; /**< Distance between the wheels [m] */
} kinematics_config_t;

/**
 * @brief Initializes the kinematics module with the given configuration.
 *
 * @param config Pointer to the kinematics configuration structure.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t kinematics_init(const kinematics_config_t *config);

/**
 * @brief Calculates forward kinematics and odometry for a differential drive robot.
 *
 * This function computes the robot's pose and velocity in the global frame based on the wheel velocities.
 *
 * The calculations are based on the following equations:
 * - Forward kinematics in the robot frame:
 * \f[{}^{R}\dot{\xi}_{R} = {}^{R}J_{W}\left(q\right)\cdot{}\dot{q} \f]
 * - Rotation from robot frame to global frame:
 * \f[{}^{G}\dot{\xi}_{R} = {}^{G}R_{R}\cdot{}^{R}\dot{\xi}_{R} \f]
 * - Integrate velocities to update the robot pose in the global frame (odometry):
 * \f[{}^{G}\xi_{R} = \int{}^{G}\dot{\xi}_{R} dt + {}^{G}\xi_{R,0} \f]
 *
 * @param qd Pointer to the wheel velocities [rad/s].
 *           Index 0: left wheel, index 1: right wheel.
 * @param xi Pointer to the robot pose [x, y, phi] in the global frame [m, m, rad].
 * @param xid Pointer to the robot velocities [vx, vy, omega] in the global frame [m/s, m/s, rad/s].
 */
void kinematics_calc_fw_kin_odom(const float qd[2], float xi[3], float xid[3]);

/**
 * @brief Calculates inverse kinematics for a differential drive robot.
 *
 * This function computes the required wheel velocities to achieve the desired robot velocities in the robot frame.
 * The calculations are based on the following equations:
 * \f[\dot{q} = {}^{W}J_{R}\left(q\right)\cdot{}{}^R\dot{\xi}_{R} \f]
 *
 * @param xid_d Pointer to the desired robot velocities [vx, vy, omega] in the robot frame.
 * @param qd_d Pointer to the required wheel velocities [rad/s].
 *             Index 0: left wheel, index 1: right wheel.
 */
void kinematics_calc_inv_kin(const float xid_d[3], float qd_d[2]);
