/**
 * @file common_math.c
 * @brief Common mathematical functions implementation file.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "common_math.h"
#include <math.h>

void derivative(const float *current, float *previous, const float *dt, float *ret, uint8_t dim)
{
    for (uint8_t i = 0; i < dim; i++)
    {
        ret[i] = (current[i] - previous[i]) / (*dt);
        previous[i] = current[i];
    }
}

void integral(float *x, const float *xd, const float *dt, uint8_t dim)
{
    for (uint8_t i = 0; i < dim; i++)
    {
        x[i] += xd[i] * (*dt);
    }
}

void matrix_vector_multiplication(const float *matrix, const float *vector, float *ret, uint8_t rows, uint8_t cols)
{
    for (uint8_t i = 0; i < rows; i++)
    {
        ret[i] = 0.0f;
        for (uint8_t j = 0; j < cols; j++)
        {
            ret[i] += matrix[i * cols + j] * vector[j];
        }
    }
}

void constrain_angle_to_pm_pi(float *angle)
{
    *angle = atan2f(sinf(*angle), cosf(*angle));
}
