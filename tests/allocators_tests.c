#include "minunit.h"

#include "../src/core/memory.h"

static const size_t f64_size = sizeof(f64);
static const size_t SIZE = 12;
static cord_bump_t *bump_allocator = NULL;
static cord_pool_t *pool_allocator = NULL;

void test_setup(void) {
    bump_allocator = cord_bump_create_with_size(SIZE);
    pool_allocator = cord_pool_create_with_size(SIZE);
}

void test_teardown(void) {
    cord_bump_destroy(bump_allocator);
    cord_pool_destroy(pool_allocator);
    bump_allocator = NULL;
    pool_allocator = NULL;
}

MU_TEST(test_cord_bump_remaining_memory) {
    size_t remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, SIZE);
    
    balloc(bump_allocator, f64_size);
    size_t expected = SIZE - f64_size;
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    // This allocation should trigger a resize (12Kb to 24Kb)
    balloc(bump_allocator, f64_size);
    expected = (2 * SIZE) - (2 * f64_size);
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    balloc(bump_allocator, f64_size);
    expected = (2 * SIZE) - (3 * f64_size);
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    // This allocation should trigger a resize (24Kb to 48Kb)
    // WRONG FIX
    // balloc(bump_allocator, 1);
    // expected = (4 * SIZE) - (3 * f64_size) + 1;
    // remaining_memory = cord_bump_remaining_memory(bump_allocator);
    // mu_assert_int_eq(expected, remaining_memory);
}

MU_TEST(test_cord_bump_resizing) {
    f64 *number = balloc(bump_allocator, f64_size);
    *number = 10.0f;
    mu_assert_int_eq(bump_allocator->capacity, SIZE);
    mu_assert_double_eq(*number, 10.0f);

    // Triggers resize
    number = balloc(bump_allocator, f64_size);
    *number = 12.0f;
    mu_assert_int_eq(bump_allocator->capacity, 2*SIZE);
    mu_assert_double_eq(*number, 12.0f);

    number = balloc(bump_allocator, f64_size);
    *number = 14.0f;
    mu_assert_int_eq(bump_allocator->capacity, 2*SIZE);
    mu_assert_double_eq(*number, 14.0f);

    // Triggers resize
    number = balloc(bump_allocator, f64_size);
    *number = 16.0f;
    mu_assert_int_eq(bump_allocator->capacity, 4*SIZE);
    mu_assert_double_eq(*number, 16.0f);

    number = balloc(bump_allocator, f64_size);
    *number = 18.0f;
    mu_assert_int_eq(bump_allocator->capacity, 4*SIZE);
    mu_assert_double_eq(*number, 18.0f);

    number = balloc(bump_allocator, f64_size);
    *number = 18.0f;
    mu_assert_int_eq(bump_allocator->capacity, 4*SIZE);
    mu_assert_double_eq(*number, 18.0f);

    // Triggers resize
    number = balloc(bump_allocator, f64_size);
    *number = 20.0f;
    mu_assert_int_eq(bump_allocator->capacity, 8*SIZE);
    mu_assert_double_eq(*number, 20.0f);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_cord_bump_remaining_memory);
    MU_RUN_TEST(test_cord_bump_resizing);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}