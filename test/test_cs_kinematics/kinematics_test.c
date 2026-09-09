#include <unity.h>
#include <kinematics.h>

void setUp(void)
{
    const kinematics_config_t config = {
        .dt = 0.001f,
        .wheel_distance = 0.5f,
    };
    kinematics_init(&config);
}

void tearDown(void) {}

void test_fw_kin_odom(void)
{
    float qd[2] = {1.0f, 1.0f};
    float xi[3] = {0.0f, 0.0f, 0.0f};
    float xid[3] = {0.0f, 0.0f, 0.0f};
    kinematics_calc_fw_kin_odom(qd, xi, xid);
    float xid_e[3] = {1.0f, 0.0f, 0.0f};
    float xi_e[3] = {0.001f, 0.0f, 0.0f};
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_e, xid, 3, "Subtest 1");
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xi_e, xi, 3, "Subtest 2");

    qd[0] = 1.0f;
    qd[1] = -1.0f;
    kinematics_calc_fw_kin_odom(qd, xi, xid);
    xid_e[0] = 0.0f;
    xid_e[1] = 0.0f;
    xid_e[2] = -4.0f;
    xi_e[0] = 0.001f;
    xi_e[1] = 0.0f;
    xi_e[2] = -0.004f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xid_e, xid, 3, "Subtest 3");
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, xi_e, xi, 3, "Subtest 4");
}

void test_inv_kin(void)
{
    float xid_d[3] = {1.0f, 0.0f, 0.0f};
    float qd_d[2] = {0.0f, 0.0f};
    kinematics_calc_inv_kin(xid_d, qd_d);
    float qd_d_e[2] = {1.0f, 1.0f};
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, qd_d_e, qd_d, 2, "Subtest 1");

    xid_d[0] = 0.0f;
    xid_d[1] = 0.0f;
    xid_d[2] = 2.0f;
    kinematics_calc_inv_kin(xid_d, qd_d);
    qd_d_e[0] = -0.5f;
    qd_d_e[1] = 0.5f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, qd_d_e, qd_d, 2, "Subtest 2");

    xid_d[0] = 1.0f;
    xid_d[1] = 0.0f;
    xid_d[2] = 4.0f;
    kinematics_calc_inv_kin(xid_d, qd_d);
    qd_d_e[0] = -0.0f;
    qd_d_e[1] = 2.0f;
    TEST_ASSERT_FLOAT_ARRAY_WITHIN_MESSAGE(1e-5f, qd_d_e, qd_d, 2, "Subtest 3");
}

void app_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_fw_kin_odom);
    RUN_TEST(test_inv_kin);

    UNITY_END();
}
