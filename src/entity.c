
#include "private_api.h"

static
void* get_component_w_index(
    ecs_entity_info_t *info,
    int32_t index)
{
    ecs_data_t *data = info->data;
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_column_t *columns = data->columns;
    ecs_assert(columns != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(index < info->table->column_count, ECS_INVALID_COMPONENT);

    ecs_column_t *column = &columns[index];
    ecs_vector_t *data_vec = column->data;
    int16_t size = column->size; 
    
    /* If size is 0, component does not have a value. This is likely caused by
     * an application trying to call ecs_get with a tag. */
    ecs_assert(size != 0, ECS_INVALID_PARAMETER);

    /* This function should not be called if an entity does not exist in the
     * provided table. Therefore if the component is found in the table, and an
     * entity exists for it, the vector cannot be NULL */
    ecs_assert(data_vec != NULL, ECS_INTERNAL_ERROR);

    void *ptr = ecs_vector_first_t(data_vec, size, column->alignment);

    /* This could only happen when the vector is empty, which should not be
     * possible since the vector should at least have one element */
    ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);

    return ECS_OFFSET(ptr, info->row * size);
}

/* Get pointer to single component value */
static
void* get_component(
    ecs_entity_info_t *info,
    ecs_entity_t component)
{
    ecs_assert(info->table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(component != 0, ECS_INTERNAL_ERROR);
    ecs_assert(info->row >= 0, ECS_INTERNAL_ERROR);

    ecs_table_t *table = info->table;
    ecs_type_t type = table->type;

    ecs_entity_t *ids = ecs_vector_first(type, ecs_entity_t);

    /* The table column_count contains the maximum column index that actually
     * contains data. This excludes component ids that do not have data, such
     * as tags. Therefore it is faster to iterate column_count vs. all the
     * elements in the type.
     *
     * The downside of this is that the code can't always detect when an 
     * application attempts to get the value of a tag (which is not allowed). To
     * ensure consistent behavior in debug mode, the entire type is iterated as
     * this guarantees that the code will assert when attempting to obtain the
     * value of a tag. */
#ifndef NDEBUG
    int i, count = ecs_vector_count(type);
#else
    int i, count = table->column_count;
#endif

    for (i = 0; i < count; i ++) {
        if (ids[i] == component) {
            return get_component_w_index(info, i);
        }
    }

    return NULL;
}

/* Utility to compute actual row from row in record */
static
int32_t set_row_info(
    ecs_entity_info_t *info,
    int32_t row)
{
    return info->row = ecs_record_to_row(row, &info->is_watched);
}

/* Utility to set info from main stage record */
static
void set_info_from_record(
    ecs_entity_t e,
    ecs_entity_info_t *info,
    ecs_record_t *record)
{
    (void)e;
    
    ecs_assert(record != NULL, ECS_INTERNAL_ERROR);

    info->record = record;

    ecs_table_t *table = record->table;

    set_row_info(info, record->row);

    info->table = table;
    if (!info->table) {
        return;
    }

    ecs_data_t *data = ecs_table_get_data(table);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    info->data = data;

    ecs_assert(ecs_vector_count(data->entities) > info->row, 
        ECS_INTERNAL_ERROR);
}

/* Get info from main stage */
bool ecs_get_info(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_info_t *info)
{
    ecs_record_t *record = ecs_eis_get(world, entity);

    if (!record) {
        info->table = NULL;
        info->is_watched = false;
        info->record = NULL;
        return false;
    }

    set_info_from_record(entity, info, record);

    return true;
}

static
ecs_lifecycle_t *get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_entity_t real_id = ecs_component_id_from_id(world, component);
    if (real_id) {
        return ecs_get_lifecycle(world, real_id);
    } else {
        return NULL;
    }
}

static
int32_t new_entity(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * new_table)
{
    ecs_record_t *record = info->record;
    ecs_data_t *new_data = ecs_table_get_or_create_data(new_table);
    int32_t new_row;

    if (!record) {
        record = ecs_eis_get_or_create(world, entity);
    }

    new_row = ecs_table_append(
        world, new_table, new_data, entity, record, true);

    record->table = new_table;
    record->row = ecs_row_to_record(new_row, info->is_watched);

    ecs_assert(
        ecs_vector_count(new_data[0].entities) > new_row, 
        ECS_INTERNAL_ERROR);

    info->data = new_data;
    
    return new_row;
}

static
int32_t move_entity(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * src_table,
    ecs_data_t * src_data,
    int32_t src_row,
    ecs_table_t * dst_table)
{    
    ecs_data_t *dst_data = ecs_table_get_or_create_data(dst_table);
    ecs_assert(src_data != dst_data, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_is_alive(world, entity), ECS_INVALID_PARAMETER);
    ecs_assert(src_table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(src_data != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(src_row >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_vector_count(src_data->entities) > src_row, 
        ECS_INTERNAL_ERROR);

    ecs_record_t *record = info->record;
    ecs_assert(!record || record == ecs_eis_get(world, entity), 
        ECS_INTERNAL_ERROR);

    int32_t dst_row = ecs_table_append(world, dst_table, dst_data, entity, 
        record, false);

    record->table = dst_table;
    record->row = ecs_row_to_record(dst_row, info->is_watched);

    ecs_assert(ecs_vector_count(src_data->entities) > src_row, 
        ECS_INTERNAL_ERROR);

    /* Copy entity & components from src_table to dst_table */
    if (src_table->type) {
        ecs_table_move(world, entity, entity, dst_table, dst_data, dst_row, 
            src_table, src_data, src_row);          
    }
    
    ecs_table_delete(world, src_table, src_data, src_row, false);

    info->data = dst_data;

    return dst_row;
}

static
void delete_entity(
    ecs_world_t * world,
    ecs_table_t * src_table,
    ecs_data_t * src_data,
    int32_t src_row)
{
    ecs_table_delete(world, src_table, src_data, src_row, true);
}

static
void commit(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * dst_table)
{
    ecs_assert(!world->in_progress, ECS_INTERNAL_ERROR);
    
    ecs_table_t *src_table = info->table;
    if (src_table == dst_table) {
        return;
    }

    if (src_table) {
        ecs_data_t *src_data = info->data;
        ecs_assert(dst_table != NULL, ECS_INTERNAL_ERROR);

        if (dst_table) { 
            info->row = move_entity(world, entity, info, src_table, 
                src_data, info->row, dst_table);
            info->table = dst_table;
        } else {
            delete_entity(world, src_table, src_data, info->row);
            ecs_record_t *r = ecs_eis_get_or_create(world, entity);
            r->table = NULL;
            r->row = (info->is_watched == true) * -1;
        }      
    } else {        
        if (dst_table) {
            info->row = new_entity(world, entity, info, dst_table);
            info->table = dst_table;
        }        
    } 
}

static
void add_w_info(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_entity_t component)
{
    ecs_table_t *dst_table = ecs_table_traverse_add(
        world, info->table, component);

    commit(world, entity, info, dst_table);
}

static
void *get_mutable(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_t component,
    ecs_entity_info_t * info,
    bool * is_added)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER);

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        return _ecs_dense_ensure(sparse, 0, entity);
    }

    void *dst = NULL;
    if (ecs_get_info(world, entity, info) && info->table) {
        dst = get_component(info, component);
    }

    if (!dst) {
        ecs_table_t *table = info->table;

        add_w_info(world, entity, info, component);
        ecs_get_info(world, entity, info);
        ecs_assert(info->table != NULL, ECS_INTERNAL_ERROR);

        dst = get_component(info, component);
        if (is_added) {
            *is_added = table != info->table;
        }
        
        return dst;
    } else {
        if (is_added) {
            *is_added = false;
        }

        return dst;
    }
}

int32_t ecs_record_to_row(
    int32_t row, 
    bool *is_watched_out) 
{
    bool is_watched = row < 0;
    row = row * -(is_watched * 2 - 1) - 1 * (row != 0);
    *is_watched_out = is_watched;
    return row;
}

int32_t ecs_row_to_record(
    int32_t row, 
    bool is_watched) 
{
    return (row + 1) * -(is_watched * 2 - 1);
}

ecs_entity_t ecs_new_id(
    ecs_world_t *world)
{
    ecs_entity_t entity;

    entity = ecs_eis_recycle(world);

    return entity;
}

ecs_entity_t ecs_new_component_id(
    ecs_world_t *world)
{    
    if (world->stats.last_component_id >= ECS_HI_COMPONENT_ID) {
        /* If the low component ids are depleted, return a regular entity id */
        return ecs_new_id(world);
    } else {
        return world->stats.last_component_id ++;
    }
}

void ecs_add_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER);
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_add(world, stage, entity, component)) {
        return;
    }

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        _ecs_dense_ensure(sparse, 0, entity);
        ecs_defer_flush(world, stage);
        return;
    }

    ecs_entity_info_t info;
    ecs_get_info(world, entity, &info);

    ecs_table_t *src_table = info.table;
    ecs_table_t *dst_table = ecs_table_traverse_add(
        world, src_table, component);

    commit(world, entity, &info, dst_table);

    ecs_defer_flush(world, stage);
}

void ecs_remove_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER); 
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_remove(world, stage, entity, component)) {
        return;
    }

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        ecs_dense_remove(sparse, entity);
        ecs_defer_flush(world, stage);
        return;
    }

    ecs_entity_info_t info;
    ecs_get_info(world, entity, &info);

    ecs_table_t *src_table = info.table;
    ecs_table_t *dst_table = ecs_table_traverse_remove(
        world, src_table, component);

    commit(world, entity, &info, dst_table);

    ecs_defer_flush(world, stage);
}

void ecs_clear(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(entity != 0, ECS_INVALID_PARAMETER);

    ecs_stage_t *stage = ecs_get_stage(&world);
    if (ecs_defer_clear(world, stage, entity)) {
        return;
    }

    ecs_entity_info_t info;
    info.table = NULL;
    ecs_get_info(world, entity, &info);
    commit(world, entity, &info, NULL);

    ecs_defer_flush(world, stage);
}

void ecs_delete(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(entity != 0, ECS_INVALID_PARAMETER);

    ecs_stage_t *stage = ecs_get_stage(&world);
    if (ecs_defer_delete(world, stage, entity)) {
        return;
    }

    ecs_record_t *r = ecs_sparse_remove_get(
        world->store.entity_index, ecs_record_t, entity);

    if (r) {
        ecs_entity_info_t info;
        set_info_from_record(entity, &info, r);

        ecs_table_t *table = info.table;
        if (table) {
            delete_entity(world, table, info.data, info.row);
            r->table = NULL;
        }

        r->row = 0;
    }

    ecs_ptree_iter_t it = ecs_ptree_iter(world->storages);
    ecs_store_t *store;
    while ((store = ecs_ptree_next(&it, ecs_store_t))) {
        ecs_dense_t *dense;
        if ((dense = store->sparse)) {
            ecs_dense_remove(dense, entity);
        }
    }

    ecs_defer_flush(world, stage);
}

const void* ecs_get_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(world->magic == ECS_WORLD_MAGIC, ECS_INTERNAL_ERROR);

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        return _ecs_dense_get(sparse, 0, entity);
    }

    ecs_entity_info_t info;
    void *ptr = NULL;
    bool found = ecs_get_info(world, entity, &info);
    if (found) {
        if (!info.table) {
            return NULL;
        }

        ptr = get_component(&info, component);        
    }

    return ptr;
}

void* ecs_get_mut_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    bool * is_added)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    void *result;

    if (ecs_defer_set(
        world, stage, EcsOpMut, entity, component, 0, NULL, &result, is_added))
    {
        return result;
    }

    ecs_entity_info_t info;
    result = get_mutable(world, entity, component, &info, is_added);
    
    ecs_defer_flush(world, stage);

    return result;
}

void ecs_modified_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_modified(world, stage, entity, component)) {
        return;
    }
    
    ecs_defer_flush(world, stage);
}

static
ecs_entity_t assign_ptr_w_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    size_t size,
    void * ptr,
    bool is_move,
    bool notify)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    (void)notify;

    if (!entity) {
        entity = ecs_new_id(world);
    }

    if (ecs_defer_set(world, stage, EcsOpSet, entity, component, 
        ecs_from_size_t(size), ptr, NULL, NULL))
    {
        return entity;
    }

    ecs_entity_info_t info;

    void *dst = get_mutable(world, entity, component, &info, NULL);

    ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);

    if (ptr) {
        ecs_entity_t real_id = ecs_component_id_from_id(world, component);
        ecs_lifecycle_t *cdata = get_lifecycle(world, real_id);
        if (cdata) {
            if (is_move) {
                ecs_move_t move = cdata->move;
                if (move) {
                    move(dst, ptr, size, 1);
                } else {
                    ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
                }
            } else {
                ecs_copy_t copy = cdata->copy;
                if (copy) {
                    copy(dst, ptr, size, 1);
                } else {
                    ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
                }
            }
        } else {
            ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
        }
    } else {
        memset(dst, 0, size);
    }

    ecs_defer_flush(world, stage);

    return entity;
}

ecs_entity_t ecs_set_ptr_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    const void *ptr,
    size_t size)
{
    /* Safe to cast away const: function won't modify if move arg is false */
    return assign_ptr_w_id(
        world, entity, component, size, (void*)ptr, false, true);
}

bool ecs_has_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_type_t type = ecs_get_type(world, entity);
    return ecs_type_has_entity(world, type, component);
}

const char* ecs_get_name(
    const ecs_world_t *world,
    ecs_entity_t entity)
{
    const EcsName *id = ecs_get(world, entity, EcsName);

    if (id) {
        return id->value;
    } else {
        return NULL;
    }
}

bool ecs_is_alive(
    const ecs_world_t *world,
    ecs_entity_t e)
{
    return ecs_eis_is_alive(world, e);
}

bool ecs_exists(
    const ecs_world_t *world,
    ecs_entity_t e)
{
    return ecs_eis_exists(world, e);
}

ecs_type_t ecs_get_type(
    const ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_record_t *record = NULL;

    record = ecs_eis_get(world, entity);

    ecs_table_t *table;
    if (record && (table = record->table)) {
        return table->type;
    }
    
    return NULL;
}

void ecs_defer_begin(
    ecs_world_t *world)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    
    if (world->in_progress) {
        ecs_stage_defer_begin(world, stage);
    } else {
        ecs_defer_none(world, stage);
    }
}

void ecs_defer_end(
    ecs_world_t *world)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    
    if (world->in_progress) {
        ecs_stage_defer_end(world, stage);
    } else {
        ecs_defer_flush(world, stage);
    }
}

static
void discard_op(
    ecs_defer_t * op)
{
    void *value = op->is._1.value;
    if (value) {
        ecs_os_free(value);
    }

    ecs_entity_t *components = op->components.array;
    if (components) {
        ecs_os_free(components);
    }
}

/* Leave safe section. Run all deferred commands. */
void ecs_defer_flush(
    ecs_world_t * world,
    ecs_stage_t * stage)
{
    if (!--stage->defer) {
        ecs_vector_t *defer_queue = stage->defer_queue;
        stage->defer_queue = NULL;
        if (defer_queue) {
            ecs_defer_t *ops = ecs_vector_first(defer_queue, ecs_defer_t);
            int32_t i, count = ecs_vector_count(defer_queue);
            
            for (i = 0; i < count; i ++) {
                ecs_defer_t *op = &ops[i];
                ecs_entity_t e = op->is._1.entity;

                /* If entity is no longer alive, this could be because the queue
                 * contained both a delete and a subsequent add/remove/set which
                 * should be ignored. */
                if (e && !ecs_is_alive(world, e) && ecs_eis_exists(world, e)) {
                    ecs_assert(op->kind != EcsOpNew, ECS_INTERNAL_ERROR);
                    discard_op(op);
                    continue;
                }

                if (op->components.count == 1) {
                    op->components.array = &op->component;
                }

                switch(op->kind) {
                case EcsOpNew:

                    /* Fallthrough */
                case EcsOpAdd:
                    ecs_add_id(world, e, op->component);
                    break;
                case EcsOpRemove:
                    ecs_remove_id(world, e, op->component);
                    break;
                case EcsOpSet:
                    assign_ptr_w_id(world, e, 
                        op->component, ecs_to_size_t(op->is._1.size), 
                        op->is._1.value, true, true);
                    break;
                case EcsOpMut:
                    assign_ptr_w_id(world, e, 
                        op->component, ecs_to_size_t(op->is._1.size), 
                        op->is._1.value, true, false);
                    break;
                case EcsOpModified:
                    ecs_modified_id(world, e, op->component);
                    break;
                case EcsOpDelete: {
                    ecs_delete(world, e);
                    break;
                }
                case EcsOpClear:
                    ecs_clear(world, e);
                    break;
                }

                if (op->components.count > 1) {
                    ecs_os_free(op->components.array);
                }

                if (op->is._1.value) {
                    ecs_os_free(op->is._1.value);
                }
            };

            if (defer_queue != stage->defer_merge_queue) {
                ecs_vector_free(defer_queue);
            }
        }
    }
}

static
size_t append_to_str(
    char **buffer,
    const char *str,
    size_t bytes_left,
    size_t *required)
{
    char *ptr = *buffer;

    size_t len = strlen(str);
    size_t to_write;
    if (bytes_left < len) {
        to_write = bytes_left;
        bytes_left = 0;
    } else {
        to_write = len;
        bytes_left -= len;
    }
    
    if (to_write) {
        ecs_os_memcpy(ptr, str, to_write);
    }

    (*required) += len;
    (*buffer) += to_write;

    return bytes_left;
}

static
size_t append_entity_to_str(
    ecs_world_t *world,
    char **ptr,
    ecs_entity_t entity,
    size_t bytes_left,
    size_t *required)
{
    const char *name = ecs_get_name(world, entity);
    if (name) {
        bytes_left = append_to_str(
            ptr, name, bytes_left, required);
    } else {
        char buf[14];
        ecs_os_sprintf(buf, "%u", (uint32_t)entity);
        bytes_left = append_to_str(
            ptr, buf, bytes_left, required);
    }
    return bytes_left;
}

size_t ecs_entity_str(
    ecs_world_t *world,
    ecs_entity_t entity,
    char *buffer,
    size_t buffer_len)
{
    char *ptr = buffer;
    size_t bytes_left = buffer_len - 1, required = 0;
    if (entity & ECS_ROLE) {
        bytes_left = append_entity_to_str(world, &ptr, ecs_get_role(entity), 
            bytes_left, &required);
        bytes_left = append_to_str(&ptr, "|", bytes_left, &required);
        entity = entity & ECS_ENTITY_MASK;
    }

    bytes_left = append_entity_to_str(world, &ptr, entity, bytes_left, 
        &required);

    ptr[0] = '\0';
    return required;
}

ecs_entity_t ecs_find_in_type(
    ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t component,
    ecs_entity_t role)
{
    ecs_vector_each(type, ecs_entity_t, c_ptr, {
        ecs_entity_t c = *c_ptr;

        if (role) {
            if (!ecs_has_role(c, role)) {
                continue;
            }
        }

        ecs_entity_t e = c & ECS_ENTITY_MASK;

        if (component) {
           ecs_type_t component_type = ecs_get_type(world, e);
           if (!ecs_type_has_entity(world, component_type, component)) {
               continue;
           }
        }

        return e;
    });

    return 0;
}

ecs_entity_t ecs_get_parent_w_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_type_t type = ecs_get_type(world, entity);    
    ecs_entity_t parent = ecs_find_in_type(world, type, component, EcsScope);
    return parent;
}
