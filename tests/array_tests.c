#include "minunit.h"

#include "../src/array.h"
#include <stdbool.h>

// Mock user strtucture
typedef struct user {
    int id;
    char name[12];
    float time_logged_in;
} user;

void user_set(user *self, user new) {
    self->id = new.id;
    self->time_logged_in = new.time_logged_in;
    strcpy(self->name, new.name);
}

bool user_equals(user *self, user *u) {
    if (self->id == u->id && self->time_logged_in == u->time_logged_in && (strcmp(self->name, u->name) == 0)) {
        return true;
    }
    return false;
}

static cord_pool_t *mem_pool = NULL;
static cord_array_t *users = NULL;

void test_setup(void) {
    // Creates a new memory pool with the default size and an new array
    mem_pool = cord_pool_create();
    users = cord_array_create(mem_pool, sizeof(user));
}

void test_teardown(void) {
    cord_array_destroy(users);
}

MU_TEST(test_cord_array_push) {
    user *my_user1 = cord_array_push(users);
    user *my_user2 = cord_array_push(users);
    user *my_user3 = cord_array_push(users);
    user *my_user4 = cord_array_push(users);

    user mock1 = { 1, "User 1", 30.32f };
    user mock2 = { 2, "User 2", 2.26f };
    user mock3 = { 3, "User 3", 58.63f };
    user mock4 = { 4, "User 4", 11.97f };

    user_set(my_user1, mock1);
    user_set(my_user2, mock2);
    user_set(my_user3, mock3);
    user_set(my_user4, mock4);

    mu_assert(my_user1, "cord_array_push() shouldn't return NULL");
    mu_assert(my_user2, "cord_array_push() shouldn't return NULL");
    mu_assert(my_user3, "cord_array_push() shouldn't return NULL");
    mu_assert(my_user4, "cord_array_push() shouldn't return NULL");
}

MU_TEST(test_cord_array_get) {
    mu_assert(user_equals(cord_array_get(users, 0), &(user){ 1, "User 1", 30.32f }), "Users should be equal");
    mu_assert(user_equals(cord_array_get(users, 1), &(user){ 2, "User 2", 2.26f }), "Users should be equal");
    mu_assert(user_equals(cord_array_get(users, 2), &(user){ 3, "User 3", 58.63f }), "Users should be equal");
    mu_assert(user_equals(cord_array_get(users, 3), &(user){ 4, "User 4", 11.97f }), "Users should be equal");
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_cord_array_push);
    MU_RUN_TEST(test_cord_array_get);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}