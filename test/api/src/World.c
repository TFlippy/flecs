#include <base.h>

void World_setup() {
    // Implement testcase
}

void World_create_delete() {
    ecs_world_t *world = ecs_init();
    ecs_fini(world);

    /* Simple test to verify if world can be created & deleted without 
     * crashing or leaking memory */

    test_assert(true);
}

void World_create_delete_w_sparse() {
    ecs_world_t *world = ecs_init();

    ecs_entity_t c = ecs_new_component_id(world);
    ecs_make_sparse(world, c);

    ecs_fini(world);

    /* Simple test to verify if world with sparse component can be created & 
     * deleted without crashing or leaking memory */

    test_assert(true);
}
