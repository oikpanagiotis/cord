#include "minunit.h"

#include "../src/core/strings.h"
#include "../src/core/typedefs.h"

#include <stdbool.h>
#include <string.h>

static inline bool streq(const char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}

#define TEST_SIZE KB(16)

/*
 *   Test cases related to cord_str_t
 */
MU_TEST(test_cord_str_equality) {
    string_ref rick = "Rick";
    string_ref morty = "Morty";
    cord_str_t rick_name = cstr(rick);
    cord_str_t morty_name = cstr(morty);

    mu_assert_string_eq(rick_name.data, rick);
    mu_assert_string_eq(morty_name.data, morty);
    mu_assert(cord_str_equals_ignore_case(rick_name, cstr("rick")),
              "Rick should be equal to rick when tested with ignore case");
    mu_assert(cord_str_equals_ignore_case(morty_name, cstr("morty")),
              "Morty should be equal to rick when tested with ignore case");
}

MU_TEST(test_cord_str_contains) {
    cord_str_t sentence = cstr("Hello everyone, how was your day?");
    mu_assert(cord_str_contains(sentence, cstr("everyone")),
              "Word everyone is contained in sentence");
}

MU_TEST(test_cord_str_substring) {
    cord_str_t date = cstr("24/12/2022");
    cord_str_t month = cord_str_substring(date, 3, 5);
    cord_str_t expected = cstr("12");
    mu_assert(cord_str_equals(month, expected), "month should be equal to 12");
}

MU_TEST(test_cord_str_trim) {
    cord_str_t trimmed = cord_str_trim(cstr("\nPanos "));
    cord_str_t expected = cstr("Panos");
    mu_assert(cord_str_equals(trimmed, expected),
              "trimmed output should be equal to Panos");

    trimmed = cord_str_trim(cstr("\t\tPanos\n"));
    mu_assert(cord_str_equals(trimmed, expected),
              "trimmed output should be equal to Panos");
}

MU_TEST(test_cord_str_remove_prefix_and_suffix) {
    cord_str_t input1 = cstr("prefix_word");
    cord_str_t input2 = cstr("word_suffix");
    cord_str_t processed1 = cord_str_remove_prefix(input1, cstr("prefix_"));
    cord_str_t processed2 = cord_str_remove_suffix(input2, cstr("_suffix"));
    cord_str_t expected = cstr("word");
    mu_assert(cord_str_equals(processed1, expected),
              "The result should be word after removing prefix");

    mu_assert(cord_str_equals(processed2, expected),
              "The result should be word after removing suffix");
}

MU_TEST(test_cord_str_first_char_and_last_char) {
    cord_str_t string = cstr("abcdef");
    char first = cord_str_first_char(string);
    char last = cord_str_last_char(string);
    mu_assert(first == 'a',
              "The first character of the string should be equal to a");
    mu_assert(last == 'f',
              "The last character of the string should be equal to a");
}

MU_TEST(test_cord_str_pop_first_split) {
    cord_str_t date = cstr("24/12/2022");
    cord_str_t expected_day = cstr("24");
    cord_str_t expected_month = cstr("12");
    cord_str_t expected_year = cstr("2022");

    cord_str_t day = cord_str_pop_first_split(&date, cstr("/"));
    cord_str_t month = cord_str_pop_first_split(&date, cstr("/"));
    cord_str_t year = cord_str_pop_first_split(&date, cstr("/"));

    mu_assert(cord_str_equals(day, expected_day), "Day should be equal to 24");
    mu_assert(cord_str_equals(month, expected_month),
              "Month should be equal to 12");
    mu_assert(cord_str_equals(year, expected_year),
              "Year should be equal to 2022");
}

/*
 *   Test cases related to cord_strbuf_t
 */
MU_TEST(test_cord_strbuf_create) {
    cord_strbuf_t *builder = cord_strbuf_create_with_size(TEST_SIZE);
    bool result = cord_strbuf_valid(builder);
    bool expected = true;
    mu_assert(result == expected,
              "After creation string builder should not be null");

    cord_strbuf_t invalid = (cord_strbuf_t){NULL, 0, 0};
    mu_assert(cord_strbuf_valid(&invalid) == false,
              "Passing invalid string builder to cord_strbuf_valid should "
              "return false");
}

MU_TEST(test_cord_strbuf_append) {
    cord_str_t expected1 = cstr("Hello");
    cord_str_t expected2 = cstr("Hello world");
    cord_strbuf_t *builder = cord_strbuf_create_with_size(TEST_SIZE);

    cord_strbuf_append(builder, cstr("Hello"));
    cord_str_t result_string = cord_strbuf_to_str(*builder);
    mu_assert(
        cord_str_equals(result_string, expected1),
        "Converting string builder to string view should result in \"Hello\"");

    cord_strbuf_append(builder, cstr(" "));
    cord_strbuf_append(builder, cstr("world"));
    result_string = cord_strbuf_to_str(*builder);

    mu_assert_int_eq(expected2.length, result_string.length);
    mu_assert(cord_str_equals(result_string, expected2),
              "Converting string builder to string view should result in "
              "\"Hello world\"");
}

MU_TEST(test_cord_strbuf_to_str) {
    cord_str_t expected = cstr("Hello");

    cord_strbuf_t *builder = cord_strbuf_create();
    cord_strbuf_append(builder, cstr("Hello"));

    cord_str_t string = cord_strbuf_to_str(*builder);
    mu_assert_int_eq(expected.length, string.length);
    mu_assert(
        cord_str_equals(string, expected),
        "Converting string builder to string view should result in \"Hello\"");
}

MU_TEST(test_cord_strbuf_to_cstring) {
    const char *test_string = "my test string";

    cord_strbuf_t *builder = cord_strbuf_create();
    cord_strbuf_append(builder, cstr(test_string));

    char *cstring = cord_strbuf_to_cstring(*builder);
    mu_assert_string_eq(test_string, cstring);

    cord_strbuf_append(builder, cstr(" edited"));
    char *edited_cstring = cord_strbuf_to_cstring(*builder);
    mu_assert_string_eq("my test string edited", edited_cstring);

    free(cstring);
    free(edited_cstring);
}

/*
 *   Test cases related to various string utilities
 */
MU_TEST(test_cstring_is_empty) {
    const char *non_empty_cstring = "my string";
    const char *empty_cstring = "";

    bool is_not_empty = !cstring_is_empty(non_empty_cstring);
    mu_assert(is_not_empty, "is_not_empty should be true");

    bool is_empty = cstring_is_empty(empty_cstring);
    mu_assert(is_empty, "is_empty should be true");
}

MU_TEST(test_cstring_is_equal) {
    const char *same1 = "same";
    const char *same2 = "same";
    const char *different = "different";

    bool are_the_same = cstring_is_equal(same1, same2);
    mu_assert(are_the_same, "same1 should be equal to same2");

    bool are_not_the_same = !cstring_is_equal(same1, different);
    mu_assert(are_not_the_same, "same1 should not be equal to different");
}

MU_TEST(test_cstring_is_null_or_empty) {
    const char *empty = "";
    const char *null = NULL;
    const char *normal = "normal";

    bool expected = true;
    bool result = cstring_is_null_or_empty(empty);
    mu_assert(expected == result,
              "cstring_is_null_or_empty should return true "
              "when passing empty string");

    expected = true;
    result = cstring_is_null_or_empty(null);
    mu_assert(expected == result,
              "cstring_is_null_or_empty should return true when passing null");

    expected = false;
    result = cstring_is_null_or_empty(normal);
    mu_assert(expected == result,
              "cstring_is_null_or_empty should return "
              "false when passing normal string");
}

MU_TEST(test_bool_to_cstring) {
    const char *expected_true = "true";
    const char *expected_false = "false";

    bool is_result_true = streq(expected_true, bool_to_cstring(true));
    mu_assert(is_result_true, "is_result_true should be true");

    bool is_result_false = streq(expected_false, bool_to_cstring(false));
    mu_assert(is_result_false, "is_result_false should be false");
}

MU_TEST(test_not_null_cstring) {
    const char *expected = "";
    char *input = NULL;
    char *input2 = "2";

    char *result = not_null_cstring(input);
    mu_assert(streq(result, expected), "result should be equal to expected");

    result = not_null_cstring(input2);
    mu_assert(streq(result, input2), "result should be equal to input2");
}

MU_TEST(test_not_null_cstring_dash) {
    const char *expected = "-";
    char *input = NULL;
    char *input2 = "2";

    const char *result = not_null_cstring_dash(input);
    mu_assert(streq(result, expected), "result should be equal to expected");

    result = not_null_cstring_dash(input2);
    mu_assert(streq(result, input2), "result should be equal to input2");
}

MU_TEST_SUITE(test_string_slice) {

    MU_RUN_TEST(test_cord_str_equality);
    MU_RUN_TEST(test_cord_str_contains);
    MU_RUN_TEST(test_cord_str_substring);
    MU_RUN_TEST(test_cord_str_trim);
    MU_RUN_TEST(test_cord_str_remove_prefix_and_suffix);
    MU_RUN_TEST(test_cord_str_first_char_and_last_char);
    MU_RUN_TEST(test_cord_str_pop_first_split);
}

MU_TEST_SUITE(test_string_builder) {

    MU_RUN_TEST(test_cord_strbuf_create);
    MU_RUN_TEST(test_cord_strbuf_append);
    MU_RUN_TEST(test_cord_strbuf_to_str);
    MU_RUN_TEST(test_cord_strbuf_to_cstring);
}

MU_TEST_SUITE(test_string_utils) {

    MU_RUN_TEST(test_cstring_is_empty);
    MU_RUN_TEST(test_cstring_is_equal);
    MU_RUN_TEST(test_cstring_is_null_or_empty);
    MU_RUN_TEST(test_bool_to_cstring);
    MU_RUN_TEST(test_not_null_cstring);
    MU_RUN_TEST(test_not_null_cstring_dash);
}

int main(void) {
    MU_RUN_SUITE(test_string_slice);
    MU_RUN_SUITE(test_string_builder);
    MU_RUN_SUITE(test_string_utils);
    MU_REPORT();
    return MU_EXIT_CODE;
}
