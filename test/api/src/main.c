
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <base.h>

// Testsuite 'World'
void World_setup(void);
void World_create_delete(void);
void World_create_delete_w_sparse(void);

bake_test_case World_testcases[] = {
    {
        "create_delete",
        World_create_delete
    },
    {
        "create_delete_w_sparse",
        World_create_delete_w_sparse
    }
};

static bake_test_suite suites[] = {
    {
        "World",
        World_setup,
        NULL,
        2,
        World_testcases
    }
};

int main(int argc, char *argv[]) {
    ut_init(argv[0]);
    return bake_test_run("base", argc, argv, suites, 1);
}
