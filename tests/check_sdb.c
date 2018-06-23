//
// Created by sam on 22/06/18.
//

#include <stdlib.h>
#include <check.h>
#include "bplus_tree_test.h"

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = blus_tree_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}