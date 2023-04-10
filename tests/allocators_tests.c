#include "minunit.h"

#include "../src/core/memory.h"

static const size_t f64_size = sizeof(f64);
static const size_t SIZE = KB(1);
static cord_bump_t *bump_allocator = NULL;
static cord_pool_t *pool_allocator = NULL;

void test_setup(void) {
    bump_allocator = cord_bump_create_with_size(SIZE);
    // pool_allocator = cord_pool_create_with_size(SIZE);
}

void test_teardown(void) {
    cord_bump_destroy(bump_allocator);
    // cord_pool_destroy(pool_allocator);
    bump_allocator = NULL;
    pool_allocator = NULL;
}

static void assert_balloc_memory_and_set(void *memory, f64 value) {
    mu_assert(memory, "Memory allocated with balloc() should not be null");
    *(f64 *)memory = value;
}

MU_TEST(test_cord_bump_remaining_memory) {
    size_t remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, SIZE);

    balloc(bump_allocator, f64_size);
    size_t expected = SIZE - f64_size;
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    balloc(bump_allocator, f64_size);
    expected = SIZE - (2 * f64_size);
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    balloc(bump_allocator, f64_size);
    expected = SIZE - (3 * f64_size);
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(remaining_memory, expected);

    balloc(bump_allocator, 1);
    expected = SIZE - ((3 * f64_size) + 1);
    remaining_memory = cord_bump_remaining_memory(bump_allocator);
    mu_assert_int_eq(expected, remaining_memory);
}

MU_TEST(test_cord_bump_memory_correctness) {
    f64 *ten = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(ten, 10.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    f64 *tewlve = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(tewlve, 12.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    f64 *fourteen = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(fourteen, 14.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    f64 *sixteen = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(sixteen, 16.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    f64 *eighteen = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(eighteen, 18.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    f64 *twenty = balloc(bump_allocator, f64_size);
    assert_balloc_memory_and_set(twenty, 20.0);
    mu_assert_int_eq(bump_allocator->capacity, SIZE);

    mu_assert_double_eq(10.0, *ten);
    mu_assert_double_eq(12.0, *tewlve);
    mu_assert_double_eq(14.0, *fourteen);
    mu_assert_double_eq(16.0, *sixteen);
    mu_assert_double_eq(18.0, *eighteen);
    mu_assert_double_eq(20.0, *twenty);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_cord_bump_memory_correctness);
    MU_RUN_TEST(test_cord_bump_remaining_memory);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}