#include <unity.h>
#include <pd_controller.h>

void setUp(void)
{
    static const float M[2][2] = {{1.0f, 0.0f}, {0.5f, 1.0f}};
    const pd_controller_config_t config = {
        .dt = 0.001f,
        .kp = 5.0f,
        .kd = 0.002f,
        .M = (const float *)M,
        .dim = 2,
    };
    pd_controller_init(&config);
}

void tearDown(void)
{
    pd_controller_cleanup();
}

void test_pd_controller_calc_Q(void)
{
    float qd[2] = {0.0f, 0.0f};
    float qd_d[2] = {1.0f, 0.0f};
    float Q[2] = {0.0f, 0.0f};
    pd_controller_calc_Q(qd, qd_d, Q);
    float Q_e[2] = {7.0f, 3.5f};
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, Q_e, Q, 2, "Subtest 1");

    pd_controller_calc_Q(qd, qd_d, Q);
    Q_e[0] = 5.0f;
    Q_e[1] = 2.5f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, Q_e, Q, 2, "Subtest 2");
}

void app_main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_pd_controller_calc_Q);
    
    UNITY_END();
}
