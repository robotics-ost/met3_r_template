#include <unity.h>
#include <common_math.h>

void setUp(void) {}

void tearDown(void) {}

void test_derivative_function(void)
{
    float current = 5.0f;
    float previous = 3.0f;
    float dt = 0.001f;
    float expected_derivative = 2000.0f;
    float calculated_derivative = 0.0f;

    derivative(&current, &previous, &dt, &calculated_derivative, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, "Subtest 1");
    TEST_ASSERT_EQUAL_FLOAT(current, previous); // previous should now be updated to current

    current = 7.0f;
    expected_derivative = 2000.0f;
    derivative(&current, &previous, &dt, &calculated_derivative, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, "Subtest 2");
    TEST_ASSERT_EQUAL_FLOAT(current, previous); // previous should now be updated to current

    current = 7.0f;
    expected_derivative = 0.0f;
    derivative(&current, &previous, &dt, &calculated_derivative, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, "Subtest 3");
    TEST_ASSERT_EQUAL_FLOAT(current, previous); // previous should now be updated to current

    current = -7.0f;
    expected_derivative = -14000.0f;
    derivative(&current, &previous, &dt, &calculated_derivative, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, "Subtest 4");
    TEST_ASSERT_EQUAL_FLOAT(current, previous); // previous should now be updated to current

    current = 0.0f;
    expected_derivative = 7000.0f;
    derivative(&current, &previous, &dt, &calculated_derivative, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, "Subtest 5");
    TEST_ASSERT_EQUAL_FLOAT(current, previous); // previous should now be updated to current
}

void test_derivative_function_2d(void)
{
    float current[2] = {5.0f, 10.0f};
    float previous[2] = {3.0f, 8.0f};
    float dt = 0.001f;
    float expected_derivative[2] = {2000.0f, 2000.0f};
    float calculated_derivative[2] = {0.0f, 0.0f};

    derivative(current, previous, &dt, calculated_derivative, 2);
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(0.01f, expected_derivative, calculated_derivative, 2, "Subtest 1");
    TEST_ASSERT_EQUAL_FLOAT_ARRAY_MESSAGE(current, previous, 2, "Subtest 2"); // previous should now be updated to current
}

void test_integral_function(void)
{
    float x = 0.0f;
    float xd = 2.0f;
    float dt = 0.001f;
    float expected_integral = 2e-3f;

    integral(&x, &xd, &dt, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-6f, expected_integral, x, "Subtest 1");

    xd = -3.0f;
    expected_integral = -1e-3f;
    integral(&x, &xd, &dt, 1);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-6f, expected_integral, x, "Subtest 2");
}

void test_integral_function_2d(void)
{
    float x[2] = {0.0f, 0.0f};
    float xd[2] = {2.0f, -3.0f};
    float dt = 0.001f;
    float expected_integral[2] = {2e-3f, -3e-3f};

    integral(x, xd, &dt, 2);
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-6f, expected_integral, x, 2, "Subtest 1");

    xd[0] = -3.0f;
    xd[1] = 4.0f;
    expected_integral[0] = -1e-3f;
    expected_integral[1] = 1e-3f;
    integral(x, xd, &dt, 2);
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-6f, expected_integral, x, 2, "Subtest 2");
}

void test_matrix_vector_multiplication(void)
{
    float matrix[4] = {1.0f, 2.0f, 3.0f, 4.0f}; // 2x2 matrix
    float vector[2] = {5.0f, 6.0f};             // 2x1 vector
    float result[2] = {0.0f, 0.0f};
    float expected_result[2] = {17.0f, 39.0f}; // Expected result of the multiplication

    matrix_vector_multiplication(matrix, vector, result, 2, 2);
    TEST_ASSERT_FLOAT_ARRAY_WITHIN(1e-5f, expected_result, result, 2);
}

void test_constrain_angle_to_pm_pi(void)
{
    float angle = 4.0f; // Example angle in radians
    float expected_angle = -2.28318530718f; // Expected result after constraining to [-pi, pi]

    constrain_angle_to_pm_pi(&angle);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-5f, expected_angle, angle, "Subtest 1");
    
    angle = -4.0f; // Example angle in radians
    expected_angle = 2.28318530718f; // Expected result after constraining
    constrain_angle_to_pm_pi(&angle);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-5f, expected_angle, angle, "Subtest 2");

    angle = 3.14f; // Example angle in radians
    expected_angle = 3.14f; // Expected result after constraining
    constrain_angle_to_pm_pi(&angle);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-5f, expected_angle, angle, "Subtest 3");
}

void app_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_derivative_function);
    RUN_TEST(test_derivative_function_2d);
    RUN_TEST(test_integral_function);
    RUN_TEST(test_matrix_vector_multiplication);
    RUN_TEST(test_constrain_angle_to_pm_pi);
    
    UNITY_END();
}
