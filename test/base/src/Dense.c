#include <base.h>

void Dense_setup() {
    ecs_os_set_api_defaults();
}

void Dense_get_or_create() {
    ecs_dense_t *d = ecs_dense_new(double);

    double *ptr = ecs_dense_ensure(d, double, 1);
    test_assert(ptr != NULL);
    *ptr = 10.5;

    ptr = ecs_dense_get(d, double, 1);
    test_assert(ptr != NULL);
    test_flt(*ptr, 10.5);

    ecs_dense_free(d);
}

void Dense_get_or_create_exists() {
    ecs_dense_t *d = ecs_dense_new(double);

    double *ptr = ecs_dense_ensure(d, double, 1);
    test_assert(ptr != NULL);
    *ptr = 10.5;

    test_assert(ptr == ecs_dense_ensure(d, double, 1));

    ecs_dense_free(d);
}

void Dense_get_not_exist() {
    ecs_dense_t *d = ecs_dense_new(double);

    test_assert(ecs_dense_get(d, double, 1) == NULL);

    ecs_dense_free(d);
}

void Dense_remove() {
    ecs_dense_t *d = ecs_dense_new(double);

    double *ptr = ecs_dense_ensure(d, double, 1);
    test_assert(ptr != NULL);
    
    test_assert(ecs_dense_get(d, double, 1) != NULL);

    ecs_dense_remove(d, 1);

    test_assert(ecs_dense_get(d, double, 1) == NULL);

    ecs_dense_free(d);
}

void Dense_remove_not_exists() {
    ecs_dense_t *d = ecs_dense_new(double);

    double *ptr = ecs_dense_ensure(d, double, 1);
    test_assert(ptr != NULL);
    
    test_assert(ecs_dense_get(d, double, 1) != NULL);

    ecs_dense_remove(d, 2);

    test_assert(ecs_dense_get(d, double, 1) != NULL);

    ecs_dense_free(d);
}

void Dense_get_0_size() {
    ecs_dense_t *d = _ecs_dense_new(0, 0);

    test_assert(_ecs_dense_ensure(d, 0, 1) == NULL);
    test_assert(_ecs_dense_get(d, 0, 1) == NULL);

    ecs_dense_free(d);
}
