#include <unity.h>
#include <pi_controller.h>

void setUp(void)
{
    static const float M[2][2] = {{1.0f, 0.0f}, {0.5f, 1.0f}};
    const pi_controller_config_t config = {
        .dt = 0.001f,
        .kp = 2.0f,
        .ki = 1000.0f,
        .integral_limit = 10.0f,
        .M = (const float *)M,
        .dim = 2,
    };
    pi_controller_init(&config);
    pi_controller_enable_integrator();
}

void tearDown(void)
{
    pi_controller_cleanup();
}

void test_pi_controller_calc_Q(void)
{
    float qd[2] = {0.0f, 0.0f};
    float qd_d[2] = {1.0f, 0.0f};
    float Q[2] = {0.0f, 0.0f};
    pi_controller_calc_Q(qd, qd_d, Q);
    float Q_e[2] = {3.0f, 1.5f};
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, Q_e, Q, 2, "Subtest 1");

    pi_controller_calc_Q(qd, qd_d, Q);
    Q_e[0] = 4.0f;
    Q_e[1] = 2.0f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, Q_e, Q, 2, "Subtest 2");
}

void app_main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_pi_controller_calc_Q);
    
    UNITY_END();
}
