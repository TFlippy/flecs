#include "private_api.h"

static
void ctor_init_zero(
    void *ptr,
    size_t size,
    int32_t count)
{
    ecs_os_memset(ptr, 0, ecs_from_size_t(size) * count);
}

static
void init_partition(ecs_world_t *world, ecs_partition_t *p) {    
    /* Initialize entity index */
    p->entity_index = ecs_sparse_new(ecs_record_t);
    ecs_sparse_set_id_source(p->entity_index, &world->stats.last_id);

    /* Initialize root table */
    p->tables = ecs_sparse_new(ecs_table_t);

    /* Initialize table map */
    p->table_map = ecs_map_new(ecs_table_t*, 8);

    /* Initialize one root table per stage */
    ecs_init_root_table(world, &p->root);
}

ecs_world_t *ecs_init(void) {
    ecs_os_init();

    ecs_trace_1("bootstrap");
    ecs_log_push();

    if (!ecs_os_has_heap()) {
        ecs_abort(ECS_MISSING_OS_API);
    }

    if (!ecs_os_has_threading()) {
        ecs_trace_1("threading not available");
    }

    if (!ecs_os_has_time()) {
        ecs_trace_1("time management not available");
    }

    ecs_world_t *world = ecs_os_calloc(sizeof(ecs_world_t));
    ecs_assert(world != NULL, ECS_OUT_OF_MEMORY);

    world->magic = ECS_WORLD_MAGIC;
    world->lc_hi = ecs_map_new(ecs_lifecycle_t, 0);  
    world->storages = ecs_ptree_new(ecs_store_t);
    init_partition(world, &world->store);
    ecs_stage_init(world, &world->stage);
    ecs_stage_init(world, &world->temp_stage);

    world->stage.world = world;
    world->temp_stage.world = world;

    ecs_bootstrap(world);

    ecs_log_pop();

    return world;
}

static
void fini_tables(
    ecs_world_t *world,
    ecs_partition_t *p)
{
    int32_t i, count = ecs_sparse_count(p->tables);

    for (i = 0; i < count; i ++) {
        ecs_table_t *t = ecs_sparse_get_elem(p->tables, ecs_table_t, i);
        ecs_table_free(world, t);
    }

    ecs_table_free(world, &p->root);
}

static
void fini_partition(
    ecs_world_t *world,
    ecs_partition_t *p)
{
    fini_tables(world, p);
    ecs_sparse_free(p->tables);
    ecs_sparse_free(p->entity_index);
    ecs_map_free(p->table_map);
}

static
void fini_storages(
    ecs_world_t *world)
{
    ecs_ptree_iter_t it = ecs_ptree_iter(world->storages);
    ecs_store_t *store;
    while ((store = ecs_ptree_next(&it, ecs_store_t))) {
        if (store->sparse) {
            ecs_dense_free(store->sparse);
        }
    }

    ecs_ptree_free(world->storages);
}

/* Cleanup component lifecycle callbacks & systems */
static
void fini_component_lifecycle(
    ecs_world_t *world)
{
    ecs_map_free(world->lc_hi);
}

/* Cleanup stages */
static
void fini_stages(
    ecs_world_t *world)
{
    ecs_stage_deinit(world, &world->stage);
    ecs_stage_deinit(world, &world->temp_stage);
}

/* The destroyer of worlds */
int ecs_fini(
    ecs_world_t *world)
{
    assert(world->magic == ECS_WORLD_MAGIC);
    assert(!world->in_progress);

    fini_stages(world);

    fini_partition(world, &world->store);
    fini_storages(world);

    fini_component_lifecycle(world);

    /* In case the application tries to use the memory of the freed world, this
     * will trigger an assert */
    world->magic = 0;

    ecs_increase_timer_resolution(0);

    /* End of the world */
    ecs_os_free(world);

    ecs_os_fini(); 

    return 0;
}

ecs_stage_t *ecs_get_stage(
    ecs_world_t **world_ptr)
{
    ecs_world_t *world = *world_ptr;

    ecs_assert(world->magic == ECS_WORLD_MAGIC ||
               world->magic == ECS_THREAD_MAGIC,
               ECS_INTERNAL_ERROR);

    if (world->magic == ECS_WORLD_MAGIC) {
        if (world->in_progress) {
            return &world->temp_stage;
        } else {
            return &world->stage;
        }
    } else if (world->magic == ECS_THREAD_MAGIC) {
        ecs_thread_t *thread = (ecs_thread_t*)world;
        *world_ptr = thread->world;
        return thread->stage;
    }
    
    return NULL;
}

int32_t ecs_get_thread_index(
    ecs_world_t *world)
{
    if (world->magic == ECS_THREAD_MAGIC) {
        ecs_thread_t *thr = (ecs_thread_t*)world;
        return thr->stage->id;
    } else if (world->magic == ECS_WORLD_MAGIC) {
        return 0;
    } else {
        ecs_abort(ECS_INTERNAL_ERROR);
    }
}

void ecs_notify_tables(
    ecs_world_t *world,
    ecs_table_event_t *event)
{
    ecs_sparse_t *tables = world->store.tables;
    int32_t i, count = ecs_sparse_count(tables);

    for (i = 0; i < count; i ++) {
        ecs_table_t *table = ecs_sparse_get_elem(tables, ecs_table_t, i);
        ecs_table_notify(world, table, event);
    }
}

void ecs_set_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component,
    ecs_lifecycle_t *lifecycle)
{
#ifndef NDEBUG
    const EcsComponent *component_ptr = ecs_get(world, component, EcsComponent);

    /* Cannot register lifecycle actions for things that aren't a component */
    ecs_assert(component_ptr != NULL, ECS_INVALID_PARAMETER);

    /* Cannot register lifecycle actions for components with size 0 */
    ecs_assert(component_ptr->size != 0, ECS_INVALID_PARAMETER);
#endif

    ecs_lifecycle_t *lc = ecs_get_or_create_lifecycle(world, component);
    ecs_assert(lc != NULL, ECS_INTERNAL_ERROR);

    if (lc->is_set) {
        ecs_assert(lc->ctor == lifecycle->ctor, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->dtor == lifecycle->dtor, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->copy == lifecycle->copy, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->move == lifecycle->move, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
    } else {
        *lc = *lifecycle;
        lc->is_set = true;

        /* If no constructor is set, invoking any of the other lifecycle actions 
         * is not safe as they will potentially access uninitialized memory. For 
         * ease of use, if no constructor is specified, set a default one that 
         * initializes the component to 0. */
        if (!lifecycle->ctor && (lifecycle->dtor || lifecycle->copy || lifecycle->move)) {
            lc->ctor = ctor_init_zero;   
        }

        ecs_notify_tables(world, &(ecs_table_event_t) {
            .kind = EcsTableComponentInfo,
            .component = component
        });
    }
}

bool ecs_component_has_actions(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_lifecycle_t *lc = ecs_get_lifecycle(world, component);
    return (lc != NULL) && lc->is_set;
}

ecs_lifecycle_t * ecs_get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_assert(component != 0, ECS_INTERNAL_ERROR);
    ecs_assert(!(component & ECS_ROLE), ECS_INTERNAL_ERROR);

    if (component < ECS_HI_COMPONENT_ID) {
        ecs_lifecycle_t *lc = &world->lc_lo[component];
        if (lc->is_set) {
            return lc;
        } else {
            return NULL;
        }
    } else {
        return ecs_map_get(world->lc_hi, ecs_lifecycle_t, component);
    }
}

ecs_lifecycle_t * ecs_get_or_create_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{    
    ecs_lifecycle_t *lc = ecs_get_lifecycle(world, component);
    if (!lc) {
        if (component < ECS_HI_COMPONENT_ID) {
            lc = &world->lc_lo[component];
        } else {
            ecs_lifecycle_t t_info = { 0 };
            ecs_map_set(world->lc_hi, component, &t_info);
            lc = ecs_map_get(world->lc_hi, ecs_lifecycle_t, component);
            ecs_assert(lc != NULL, ECS_INTERNAL_ERROR); 
        }
    }

    return lc;
}

void ecs_dim(
    ecs_world_t *world,
    int32_t entity_count)
{
    assert(world->magic == ECS_WORLD_MAGIC);
    ecs_eis_set_size(world, entity_count + ECS_HI_COMPONENT_ID);
}

ecs_entity_t ecs_set_scope(
    ecs_world_t *world,
    ecs_entity_t scope)
{
    ecs_stage_t *stage = ecs_get_stage(&world);

    ecs_entity_t e = ecs_role(EcsScope, scope);

    ecs_entity_t cur = stage->scope;
    stage->scope = scope;

    if (scope) {
        stage->scope_table = ecs_table_traverse_add(
            world, &world->store.root, e);
    } else {
        stage->scope_table = &world->store.root;
    }

    return cur;
}

ecs_entity_t ecs_get_scope(
    const ecs_world_t *world)
{
    /* Cast is safe, ecs_get_stage doesn't modify the world */
    ecs_stage_t *stage = ecs_get_stage((ecs_world_t**)&world);
    return stage->scope;
}

void ecs_make_sparse(
    ecs_world_t *world,
    ecs_entity_t id)
{
    ecs_store_t *store = ecs_ptree_ensure(
        world->storages, ecs_store_t, id);

    if (!store->sparse) {
        const EcsComponent *ptr = ecs_get(world, id, EcsComponent);
        if (ptr) {
            store->sparse = _ecs_dense_new(ptr->size, ptr->alignment);
        } else {
            store->sparse = _ecs_dense_new(0, 0);
        }
    } 
}
