#include <unity.h>
#include <position_controller_diff_drive.h>
#include <math.h>

void setUp(void)
{
    position_controller_config_t config = {
        .k1 = 1.0f,
        .k2 = 1.0f,
        .k3 = 1.0f,
        .k4 = 0.5f,
        .position_tolerance = 1e-3f,
        .orientation_tolerance = 3e-2f,
        .max_linear_velocity_x = 10.0f,
        .max_angular_velocity = 5.0f,
    };
    position_controller_init(&config);
}

void tearDown(void) {}

void test_position_controller_run(void)
{
    float xi[3] = {0.0f, 0.0f, 0.0f};
    float xid_d[3] = {0.0f, 0.0f, 0.0f};
    // Run without setting a target
    position_controller_run(xi, xid_d);
    float xid_d_e[3] = {0.0f, 0.0f, 0.0f};
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_d_e, xid_d, 3, "Subtest 1");

    float xi_d[3] = {1.0f, 0.0f, 0.0f};
    position_controller_set_target(xi_d);
    position_controller_run(xi, xid_d);
    xid_d_e[0] = 1.0f;
    xid_d_e[1] = 0.0f;
    xid_d_e[2] = 0.0f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_d_e, xid_d, 3, "Subtest 2");

    xi_d[0] = 0.0f;
    xi_d[1] = 0.0f;
    xi_d[2] = M_PI_2;
    position_controller_set_target(xi_d);
    position_controller_run(xi, xid_d);
    xid_d_e[0] = 0.0f;
    xid_d_e[1] = 0.0f;
    xid_d_e[2] = M_PI_4;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_d_e, xid_d, 3, "Subtest 3");

    xi_d[0] = 0.0f;
    xi_d[1] = 1.0f;
    xi_d[2] = M_PI_2;
    position_controller_set_target(xi_d);
    position_controller_run(xi, xid_d);
    xid_d_e[0] = 0.0f;
    xid_d_e[1] = 0.0f;
    xid_d_e[2] = M_PI_2;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_d_e, xid_d, 3, "Subtest 4");
}

void app_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_position_controller_run);

    UNITY_END();
}