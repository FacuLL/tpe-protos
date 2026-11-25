#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "users.h"

static void
count_cb(const char *name, void *data) {
    (void) name;
    (*(size_t *) data)++;
}

START_TEST (test_users_lifecycle) {
    users_init();
    ck_assert_int_eq(USERS_OK,     users_add("juan", "1234"));
    ck_assert_int_eq(USERS_EXISTS, users_add("juan", "otra"));
    ck_assert_int_eq(USERS_OK,     users_add("ana",  "abcd"));
    ck_assert_uint_eq(2, users_count());

    ck_assert(users_authenticate("juan", "1234"));
    ck_assert(!users_authenticate("juan", "mal"));
    ck_assert(!users_authenticate("nadie", "1234"));

    ck_assert_int_eq(USERS_OK, users_set_password("juan", "nueva"));
    ck_assert(!users_authenticate("juan", "1234"));
    ck_assert(users_authenticate("juan", "nueva"));

    // borrar el primero no debe perder al segundo
    ck_assert_int_eq(USERS_OK,        users_remove("juan"));
    ck_assert_int_eq(USERS_NOT_FOUND, users_remove("juan"));
    ck_assert(users_authenticate("ana", "abcd"));

    size_t n = 0;
    users_foreach(count_cb, &n);
    ck_assert_uint_eq(1, n);
    users_destroy();
    ck_assert_uint_eq(0, users_count());
}
END_TEST

START_TEST (test_users_invalid) {
    users_init();
    char largo[USERS_MAX_NAME_LEN + 2];
    memset(largo, 'a', sizeof(largo) - 1);
    largo[sizeof(largo) - 1] = '\0';   // 256 caracteres: uno de más

    ck_assert_int_eq(USERS_INVALID, users_add("", "x"));
    ck_assert_int_eq(USERS_INVALID, users_add("x", ""));
    ck_assert_int_eq(USERS_INVALID, users_add("a:b", "x"));
    ck_assert_int_eq(USERS_INVALID, users_add("a\nb", "x"));
    ck_assert_int_eq(USERS_INVALID, users_add(largo, "x"));
    largo[USERS_MAX_NAME_LEN] = '\0';  // 255: justo el máximo
    ck_assert_int_eq(USERS_OK, users_add(largo, "x"));
    users_destroy();
}
END_TEST

START_TEST (test_users_full) {
    users_init();
    char name[16];
    enum users_status st = USERS_OK;
    for (int i = 0; st == USERS_OK; i++) {
        snprintf(name, sizeof(name), "u%d", i);
        st = users_add(name, "x");
    }
    ck_assert_int_eq(USERS_FULL, st);
    users_destroy();
}
END_TEST

Suite *
suite(void) {
    Suite *s  = suite_create("users");
    TCase *tc = tcase_create("users");

    tcase_add_test(tc, test_users_lifecycle);
    tcase_add_test(tc, test_users_invalid);
    tcase_add_test(tc, test_users_full);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    SRunner *sr = srunner_create(suite());
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
