#include "minunit.h"

static int in = 0;
static int expect = 0;

void test_setup(void) {
    in = 1;
    expect = 1;
}

void test_teardown(void) {
    // ...
}

MU_TEST(test_my_assert) {
    mu_assert(in == 1, "in should be 1");
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_my_assert);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}