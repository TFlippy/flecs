#include "private_api.h"

/* Component lifecycle actions for EcsName */
void EcsName_ctor(EcsName *ptr, size_t size, int32_t count) {
    memset(ptr, 0, size * ecs_to_size_t(count));
}

void EcsName_dtor(EcsName *ptr, size_t size, int32_t count) {
    (void)size;

    int32_t i;
    for (i = 0; i < count; i ++) {
        ecs_os_free(ptr[i].alloc_value);
    }
}

void EcsName_copy(EcsName *dst, const EcsName *src, size_t size, int32_t count) {
    (void)size;
    
    int32_t i;
    for (i = 0; i < count; i ++) {
        if (dst[i].alloc_value) {
            ecs_os_free(dst[i].alloc_value);
            dst[i].alloc_value = NULL;
        }
        
        if (src[i].alloc_value) {
            dst[i].alloc_value = ecs_os_strdup(src[i].alloc_value);
            dst[i].value = dst[i].alloc_value;
        } else {
            dst[i].alloc_value = NULL;
            dst[i].value = src[i].value;
        }
        dst[i].symbol = src[i].symbol;
    }
}

void EcsName_move(EcsName *dst, EcsName *src, size_t size, int32_t count) {
    memcpy(dst, src, size * ecs_to_size_t(count));
    memset(src, 0, size * ecs_to_size_t(count));
}


/* -- Bootstrapping -- */

#define bootstrap_component(world, table, name)\
    _bootstrap_component(world, table, ecs_typeid(name), #name, sizeof(name),\
        ECS_ALIGNOF(name))

void _bootstrap_component(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t entity,
    const char *id,
    ecs_size_t size,
    ecs_size_t alignment)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);

    ecs_data_t *data = ecs_table_get_or_create_data(table);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_column_t *columns = data->columns;
    ecs_assert(columns != NULL, ECS_INTERNAL_ERROR);

    /* Create record in entity index */
    ecs_record_t *record = ecs_eis_get_or_create(world, entity);
    record->table = table;

    /* Insert row into table to store EcsComponent itself */
    int32_t index = ecs_table_append(world, table, data, entity, record, false);
    record->row = index + 1;

    /* Set size and id */
    EcsComponent *c_info = ecs_vector_first(columns[0].data, EcsComponent);
    EcsName *id_data = ecs_vector_first(columns[1].data, EcsName);
    
    c_info[index].size = size;
    c_info[index].alignment = alignment;
    id_data[index].value = &id[ecs_os_strlen("Ecs")]; /* Skip prefix */
    id_data[index].symbol = id;
    id_data[index].alloc_value = NULL;
}

/** Create type for component */
ecs_type_t ecs_bootstrap_type(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_table_t *table = ecs_table_find_or_create(world, &(ecs_entities_t){
        .array = (ecs_entity_t[]){entity},
        .count = 1
    });

    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table->type != NULL, ECS_INTERNAL_ERROR);

    return table->type;
}

/** Initialize component table. This table is manually constructed to bootstrap
 * flecs. After this function has been called, the builtin components can be
 * created. 
 * The reason this table is constructed manually is because it requires the size
 * and alignment of the EcsComponent and EcsName components, which haven't been 
 * created yet */
static
ecs_table_t* bootstrap_component_table(
    ecs_world_t *world)
{
    ecs_entity_t entities[] = {ecs_typeid(EcsComponent), ecs_typeid(EcsName), ecs_role(EcsScope, EcsFlecsCore)};
    ecs_entities_t array = {
        .array = entities,
        .count = 3
    };

    ecs_table_t *result = ecs_table_find_or_create(world, &array);
    ecs_data_t *data = ecs_table_get_or_create_data(result);

    /* Preallocate enough memory for initial components */
    data->entities = ecs_vector_new(ecs_entity_t, EcsFirstUserComponentId);
    data->record_ptrs = ecs_vector_new(ecs_record_t*, EcsFirstUserComponentId);

    data->columns = ecs_os_malloc(sizeof(ecs_column_t) * 2);
    ecs_assert(data->columns != NULL, ECS_OUT_OF_MEMORY);

    data->columns[0].data = ecs_vector_new(EcsComponent, EcsFirstUserComponentId);
    data->columns[0].size = sizeof(EcsComponent);
    data->columns[0].alignment = ECS_ALIGNOF(EcsComponent);
    data->columns[1].data = ecs_vector_new(EcsName, EcsFirstUserComponentId);
    data->columns[1].size = sizeof(EcsName);
    data->columns[1].alignment = ECS_ALIGNOF(EcsName);

    result->column_count = 2;
    
    return result;
}

void ecs_bootstrap(
    ecs_world_t *world)
{
    ecs_trace_1("bootstrap core components");
    ecs_log_push();

    /* Create table that will hold components (EcsComponent, EcsName) */
    ecs_table_t *table = bootstrap_component_table(world);
    assert(table != NULL);

    bootstrap_component(world, table, EcsComponent);
    bootstrap_component(world, table, EcsName);

    world->stats.last_component_id = EcsFirstUserComponentId;
    world->stats.last_id = EcsFirstUserEntityId;

    ecs_set_scope(world, EcsFlecsCore);

    ecs_bootstrap_tag(world, EcsModule);
    ecs_bootstrap_tag(world, EcsDisabled);
    ecs_bootstrap_tag(world, EcsWildcard);
    ecs_bootstrap_tag(world, EcsScope);

    ecs_set_lifecycle(world, ecs_typeid(EcsName), &(ecs_lifecycle_t){
        .ctor = (ecs_xtor_t)EcsName_ctor,
        .dtor = (ecs_xtor_t)EcsName_dtor,
        .copy = (ecs_copy_t)EcsName_copy,
        .move = (ecs_move_t)EcsName_move
    });

    /* Initialize scopes */
    ecs_set(world, EcsFlecs, EcsName, {.value = "flecs"});
    ecs_add_id(world, EcsFlecs, EcsModule);
    ecs_set(world, EcsFlecsCore, EcsName, {.value = "core"});
    ecs_add_id(world, EcsFlecsCore, EcsModule);
    ecs_add_id(world, EcsFlecsCore, ecs_role(EcsScope, EcsFlecs));

    ecs_set_scope(world, 0);

    ecs_log_pop();
}
