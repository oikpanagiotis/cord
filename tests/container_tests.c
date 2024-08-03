#include "minunit.h"

#include "../src/core/array.h"
#include "../src/core/hashmap.h"
#include <stdbool.h>

typedef struct user {
    int id;
    char name[12];
    float time_logged_in;
} user;

static void *create_user(cord_bump_t *alloc) {
    return balloc(alloc, sizeof(user));
}

void print_user(user *u) {
    printf("User %d %s %f\n", u->id, u->name, u->time_logged_in);
}

void user_set(user *self, user new) {
    self->id = new.id;
    self->time_logged_in = new.time_logged_in;
    strcpy(self->name, new.name);
}

bool user_equals(user *self, user *u) {
    return self->id == u->id && self->time_logged_in == u->time_logged_in &&
           strcmp(self->name, u->name) == 0;
}

static user *my_user1 = NULL;
static user *my_user2 = NULL;
static user *my_user3 = NULL;
static user *my_user4 = NULL;

static cord_bump_t *allocator = NULL;
static cord_array_t *users_list = NULL;
static cord_hashmap_t *users_map = NULL;

void cord_array_test_setup(void) {
    // Creates a new memory region with the default size and an new array
    allocator = cord_bump_create_with_size(KB(64));
    users_list = cord_array_create(allocator, sizeof(user));
}

void cord_array_test_teardown(void) {
    cord_bump_destroy(allocator);
    allocator = NULL;
    users_list = NULL;
}

void init_four_users(void) {
    my_user1 = cord_array_push(users_list);
    my_user2 = cord_array_push(users_list);
    my_user3 = cord_array_push(users_list);
    my_user4 = cord_array_push(users_list);

    user mock1 = {1, "User 1", 30.32f};
    user mock2 = {2, "User 2", 2.26f};
    user mock3 = {3, "User 3", 58.63f};
    user mock4 = {4, "User 4", 11.97f};

    user_set(my_user1, mock1);
    user_set(my_user2, mock2);
    user_set(my_user3, mock3);
    user_set(my_user4, mock4);
}

void assert_all_users_not_null(char *msg) {
    mu_assert(my_user1, msg);
    mu_assert(my_user2, msg);
    mu_assert(my_user3, msg);
    mu_assert(my_user4, msg);
}

MU_TEST(test_cord_array_push) {
    init_four_users();

    assert_all_users_not_null("cord_array_push() shouldn't return NULL");
}

MU_TEST(test_cord_array_get) {
    init_four_users();

    mu_assert(user_equals(cord_array_get(users_list, 0),
                          &(user){1, "User 1", 30.32f}),
              "Users should be equal");
    mu_assert(
        user_equals(cord_array_get(users_list, 1), &(user){2, "User 2", 2.26f}),
        "Users should be equal");
    mu_assert(user_equals(cord_array_get(users_list, 2),
                          &(user){3, "User 3", 58.63f}),
              "Users should be equal");
    mu_assert(user_equals(cord_array_get(users_list, 3),
                          &(user){4, "User 4", 11.97f}),
              "Users should be equal");
}

MU_TEST(test_cord_array_resize) {
    user mock1 = {1, "User 1", 30.32f};
    user mock4 = {4, "User 4", 11.97f};
    for (int i = 0; i < 20; i++) {
        if (i == 14) {
            my_user4 = cord_array_push(users_list);
            user_set(my_user4, mock4);
            continue;
        }

        my_user1 = cord_array_push(users_list);
    }
    user_set(my_user1, mock1);

    mu_assert(user_equals(cord_array_get(users_list, 19),
                          &(user){1, "User 1", 30.32f}),
              "Users should be equal");
    mu_assert(user_equals(cord_array_get(users_list, 14),
                          &(user){4, "User 4", 11.97f}),
              "Users should be equal");
}

void cord_hashmap_test_setup(void) {
    allocator = cord_bump_create_with_size(KB(64));
    users_map = cord_hashmap_create(allocator, create_user);
}

void cord_hashmap_test_teardown(void) {
    cord_bump_destroy(allocator);
    allocator = NULL;
    users_map = NULL;
}

void update_user(user *u, int id, char *name, float time_logged_in) {
    strcpy(u->name, name);
    u->id = id;
    u->time_logged_in = time_logged_in;
}

void assert_user_values(user *u, int id, char *name, float time_logged_in) {
    mu_assert_string_eq(name, u->name);
    mu_assert_int_eq(id, u->id);
    mu_assert_double_eq(time_logged_in, u->time_logged_in);
}

MU_TEST(test_cord_hashmap_put) {
    my_user1 = cord_hashmap_put(users_map, "first");
    my_user2 = cord_hashmap_put(users_map, "second");
    my_user3 = cord_hashmap_put(users_map, "third");
    my_user4 = cord_hashmap_put(users_map, "fourth");

    assert_all_users_not_null("cord_hashmap_put() shouldn't return NULL");

    update_user(my_user1, 1, "first", 1);
    update_user(my_user2, 2, "second", 2);
    update_user(my_user3, 3, "third", 3);
    update_user(my_user4, 4, "fourth", 4);

    assert_user_values(my_user1, 1, "first", 1);
    assert_user_values(my_user2, 2, "second", 2);
    assert_user_values(my_user3, 3, "third", 3);
    assert_user_values(my_user4, 4, "fourth", 4);
}

MU_TEST(test_cord_hashmap_get) {
    my_user1 = cord_hashmap_put(users_map, "first");
    my_user2 = cord_hashmap_put(users_map, "second");
    my_user3 = cord_hashmap_put(users_map, "third");
    my_user4 = cord_hashmap_put(users_map, "fourth");

    assert_all_users_not_null("cord_hashmap_put() shouldn't return NULL");

    update_user(my_user1, 1, "first", 1);
    update_user(my_user2, 2, "second", 2);
    update_user(my_user3, 3, "third", 3);
    update_user(my_user4, 4, "fourth", 4);

    user *returned1 = cord_hashmap_get(users_map, "first");
    user *returned2 = cord_hashmap_get(users_map, "second");
    user *returned3 = cord_hashmap_get(users_map, "third");
    user *returned4 = cord_hashmap_get(users_map, "fourth");

    assert_user_values(returned1, 1, "first", 1);
    assert_user_values(returned2, 2, "second", 2);
    assert_user_values(returned3, 3, "third", 3);
    assert_user_values(returned4, 4, "fourth", 4);
}

MU_TEST_SUITE(cord_array_test_suite) {
    MU_SUITE_CONFIGURE(&cord_array_test_setup, &cord_array_test_teardown);
    MU_RUN_TEST(test_cord_array_push);
    MU_RUN_TEST(test_cord_array_get);
    MU_RUN_TEST(test_cord_array_resize);
}

MU_TEST_SUITE(cord_hashmap_test_suite) {
    MU_SUITE_CONFIGURE(&cord_hashmap_test_setup, &cord_hashmap_test_teardown);
    MU_RUN_TEST(test_cord_hashmap_put);
    MU_RUN_TEST(test_cord_hashmap_get);
}

int main(void) {
    MU_RUN_SUITE(cord_array_test_suite);
    MU_RUN_SUITE(cord_hashmap_test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
