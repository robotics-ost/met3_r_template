#include <unity.h>
#include <motor_model.h>

void setUp(void)
{
    static const motor_model_params_t motor_params = {
        .transmission_ratio = 3441.0f / 104.0f,
        .armature_resistance = 8.0f,
        .motor_constant = 8.44e-3f,
        .max_torque_continuous = 9.0671316e-4f,
        .max_torque_intermittent = 3.02237721e-3f,
        .max_velocity = 523.098173f,
        .invert_direction = false,
    };
    const motor_model_config_t motor_config = {
        .dt = 0.001f,
        .params = &motor_params,
        .nr_of_motors = 1,
    };
    motor_model_init(&motor_config);
}

void tearDown(void)
{
    motor_model_cleanup();
}

void test_motor_model_calc_q(void)
{
    const float shaft_angle = 1.0f; // rad
    float q = 0.0f;
    motor_model_calc_q(&shaft_angle, &q);
    float q_e = 3.0223772159e-2f;
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, q_e, q);
}

void test_motor_model_calc_qd(void)
{
    const float q = 0.1f;
    float qd = 0.0f;
    float qd_e = 100.0f;
    motor_model_calc_qd(&q, &qd);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-3f, qd_e, qd, "Subtest 1");

    const float q2 = 0.3f;
    qd_e = 200.0f;
    motor_model_calc_qd(&q2, &qd);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-3f, qd_e, qd, "Subtest 2");
}

void test_motor_model_calc_voltage(void)
{
    const float Q = 0.03f;
    const float qd = 5.0f;
    float U = 0.0f;
    motor_model_calc_voltage(&Q, &qd, &U);

    float U_e = 2.25569568f;

    TEST_ASSERT_FLOAT_WITHIN(1e-6f, U_e, U);
}

void app_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_motor_model_calc_q);
    RUN_TEST(test_motor_model_calc_qd);
    RUN_TEST(test_motor_model_calc_voltage);

    UNITY_END();
}
