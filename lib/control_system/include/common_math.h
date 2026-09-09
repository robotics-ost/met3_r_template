/**
 * @file common_math.h
 * @brief Common mathematical functions header file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#pragma once

#include <stdint.h>

/**
 * @brief Calculates the derivative of a signal.
 *
 * @param current Pointer to the current value.
 * @param previous Pointer to the previous value.
 * @param dt Time step for derivative calculation [s].
 * @param ret Pointer to return the calculated derivative.
 * @param dim Dimension of the signal (e.g., 1 for scalar, 2 for vector).
 */
void derivative(const float *current, float *previous, const float *dt, float *ret, uint8_t dim);

/**
 * @brief Calculates the integral of a signal.
 *
 * @param x Pointer to the current value of the integral.
 *          This value will be updated with the new integral value.
 * @param xd Pointer to the derivative value to be integrated.
 * @param dt Time step for integration [s].
 * @param dim Dimension of the signal (e.g., 1 for scalar, 2 for vector).
 */
void integral(float *x, const float *xd, const float *dt, uint8_t dim);

/**
 * @brief Calculates the multiplication of a matrix with a vector.
 *
 * @param matrix Pointer to the matrix (row-major order).
 * @param vector Pointer to the vector.
 * @param ret Pointer to return the resulting vector.
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 */
void matrix_vector_multiplication(const float *matrix, const float *vector, float *ret, uint8_t rows, uint8_t cols);

/**
 * @brief Constrains an angle to the range [-π, π].
 *
 * @param angle Pointer to the angle value to be constrained [rad].
 */
void constrain_angle_to_pm_pi(float *angle);
