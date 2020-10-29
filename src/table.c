#include "private_api.h"

static
ecs_flags32_t get_component_action_flags(
    ecs_lifecycle_t *lc) 
{
    ecs_flags32_t flags = 0;

    if (lc->ctor) {
        flags |= EcsTableHasCtors;
    }
    if (lc->dtor) {
        flags |= EcsTableHasDtors;
    }
    if (lc->copy) {
        flags |= EcsTableHasCopy;
    }
    if (lc->move) {
        flags |= EcsTableHasMove;
    } 

    return flags;  
}

/* Check if table has instance of component, including traits */
static
bool has_component(
    ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t component)
{
    ecs_entity_t *entities = ecs_vector_first(type, ecs_entity_t);
    int32_t i, count = ecs_vector_count(type);

    for (i = 0; i < count; i ++) {
        if (component == ecs_component_id_from_id(world, entities[i])) {
            return true;
        }
    }
    
    return false;
}

static
void notify_component_info(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t component)
{
    ecs_type_t table_type = table->type;
    if (!component || has_component(world, table_type, component)){
        int32_t column_count = ecs_vector_count(table_type);
        ecs_assert(!component || column_count != 0, ECS_INTERNAL_ERROR);

        if (!column_count) {
            return;
        }

        ecs_data_t *data = ecs_table_get_data(table);
        if (!data) {
            return;
        }
        
        if (!data->lc) {
            data->lc = ecs_os_calloc(
                ECS_SIZEOF(ecs_lifecycle_t) * column_count);
        }

        /* Reset lifecycle flags before recomputing */
        table->flags &= ~EcsTableHasLifecycle;

        /* Recompute lifecycle flags */
        ecs_entity_t *array = ecs_vector_first(table_type, ecs_entity_t);
        int32_t i;
        for (i = 0; i < column_count; i ++) {
            ecs_entity_t c = ecs_component_id_from_id(world, array[i]);
            if (!c) {
                continue;
            }
            
            ecs_lifecycle_t *lc = ecs_get_lifecycle(world, c);
            if (lc) {
                ecs_flags32_t flags = get_component_action_flags(lc);
                table->flags |= flags;
                data->lc[i] = *lc;
            }            
        }        
    }
}


ecs_data_t* ecs_init_data(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *result)
{
    ecs_type_t type = table->type; 
    int32_t i, count = table->column_count;

    /* Root tables don't have columns */
    if (!count) {
        result->columns = NULL;
        return result;
    }

    /* When a table is going to store data, initialize the cache with component
     * lifecycle callbacks */
    notify_component_info(world, table, 0);

    ecs_entity_t *entities = ecs_vector_first(type, ecs_entity_t);

    if (count) {
        result->columns = ecs_os_calloc(ECS_SIZEOF(ecs_column_t) * count);    
    } else if (count) {
        /* If a table has switch columns, store vector with the case values
            * as a regular column, so it's easier to access for systems. To
            * enable this, we need to allocate more space. */
        int32_t type_count = ecs_vector_count(type);
        result->columns = ecs_os_calloc(ECS_SIZEOF(ecs_column_t) * type_count);
    }

    if (count) {
        for (i = 0; i < count; i ++) {
            ecs_entity_t e = entities[i];

            /* Is the column a component? */
            const EcsComponent *component = ecs_component_from_id(world, e);
            if (component) {
                /* Is the component associated wit a (non-empty) type? */
                if (component->size) {
                    /* This is a regular component column */
                    result->columns[i].size = ecs_to_i16(component->size);
                    result->columns[i].alignment = ecs_to_i16(component->alignment);
                } else {
                    /* This is a tag */
                }
            } else {
                /* This is an entity that was added to the type */
            }
        }
    }

    return result;
}

/* -- Private functions -- */

static
ecs_data_t* get_data_intern(
    ecs_table_t *table,
    bool create)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);

    ecs_data_t *data = table->data;
    if (data) {
        return data;
    }

    if (!data && !create) {
        return NULL;
    }

    return table->data = ecs_os_calloc(ECS_SIZEOF(ecs_data_t));
}

ecs_data_t* ecs_table_get_data(
    ecs_table_t *table)
{
    return get_data_intern(table, false);
}

ecs_data_t* ecs_table_get_or_create_data(
    ecs_table_t *table)
{
    return get_data_intern(table, true);
}

static
void ctor_component(
    ecs_lifecycle_t * lc,
    ecs_column_t * column,
    int32_t row,
    int32_t count)
{
    ecs_xtor_t ctor;
    if (lc && (ctor = lc->ctor)) {
        int16_t size = column->size;
        int16_t alignment = column->alignment;
        void *ptr = ecs_vector_get_t(column->data, size, alignment, row);
        ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);
        ctor(ptr, ecs_to_size_t(size), count);
    }
}

static
void dtor_component(
    ecs_lifecycle_t * lc,
    ecs_column_t * column,
    int32_t row,
    int32_t count)
{
    ecs_xtor_t dtor;
    if (lc && (dtor = lc->dtor)) {
        int16_t size = column->size;
        int16_t alignment = column->alignment;    
        void *ptr = ecs_vector_get_t(column->data, size, alignment, row);
        ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);
        dtor(ptr, ecs_to_size_t(size), count);
    }
}

static
void dtor_all_components(
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t row,
    int32_t count)
{
    int32_t column_count = table->column_count;
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &data->columns[i];
        dtor_component(&data->lc[i], column, row, count);
    }
}

static
void run_remove_actions(
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t row,
    int32_t count)
{
    if (count) {
        dtor_all_components(table, data, row, count);
    }
}

void ecs_table_clear_data(
    ecs_table_t * table,
    ecs_data_t * data)
{
    if (!data) {
        return;
    }
    
    ecs_column_t *columns = data->columns;
    if (columns) {
        int32_t c, column_count = table->column_count;
        for (c = 0; c < column_count; c ++) {
            ecs_vector_free(columns[c].data);
        }
        ecs_os_free(columns);
        data->columns = NULL;
    }

    ecs_vector_free(data->entities);
    ecs_vector_free(data->record_ptrs);
    ecs_os_free(data->dirty_state);

    data->entities = NULL;
    data->record_ptrs = NULL;
}

/* Clear columns. Deactivate table in systems if necessary, but do not invoke
 * OnRemove handlers. This is typically used when restoring a table to a
 * previous state. */
void ecs_table_clear_silent(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_data_t *data = ecs_table_get_data(table);
    if (!data) {
        return;
    }

    int32_t count = ecs_vector_count(data->entities);
    
    ecs_table_clear_data(table, table->data);

    if (count) {
        // ecs_table_activate(world, table, 0, false);
    }
}

/* Delete all entities in table, invoke OnRemove handlers. This function is used
 * when an application invokes delete_w_filter. Use ecs_table_clear_silent, as 
 * the table may have to be deactivated with systems. */
void ecs_table_clear(
    ecs_world_t * world,
    ecs_table_t * table)
{
    ecs_data_t *data = ecs_table_get_data(table);
    if (data) {
        run_remove_actions(table, data, 0, ecs_table_data_count(data));

        ecs_entity_t *entities = ecs_vector_first(data->entities, ecs_entity_t);
        int32_t i, count = ecs_vector_count(data->entities);
        for(i = 0; i < count; i ++) {
            ecs_eis_delete(world, entities[i]);
        }
    }

    ecs_table_clear_silent(world, table);
}

/* Free table resources. Do not invoke handlers and do not activate/deactivate
 * table with systems. This function is used when the world is freed. */
void ecs_table_free(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;
    ecs_data_t *data = ecs_table_get_data(table);
    if (data) {
        run_remove_actions(table, data, 0, ecs_table_data_count(data));
        if (data->lc) {
            ecs_os_free(data->lc);
        }            
    }

    ecs_table_clear_data(table, table->data);
    ecs_table_clear_edges(table);
    
    ecs_ptiny_free(table->edges);
    ecs_vector_free((ecs_vector_t*)table->type);

    table->id = 0;

    ecs_os_free(table->data);
}

/* Reset a table to its initial state. */
void ecs_table_reset(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_ptiny_free(table->edges);
    table->edges = ecs_ptiny_new(ecs_edge_t);
}

static
void mark_dirty(
    ecs_data_t *data,
    int32_t index)
{
    if (data->dirty_state) {
        data->dirty_state[index] ++;
    }
}

void ecs_table_mark_dirty(
    ecs_table_t *table,
    ecs_entity_t component)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table->data != NULL, ECS_INTERNAL_ERROR);
    
    ecs_data_t *data = table->data;
    if (data->dirty_state) {
        int32_t index = ecs_type_index_of(table->type, component);
        ecs_assert(index != -1, ECS_INTERNAL_ERROR);
        mark_dirty(data, index);
    }
}

static
void ensure_data(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t * column_count_out,
    ecs_column_t ** columns_out)
{
    int32_t column_count = table->column_count;
    ecs_column_t *columns = NULL;

    /* It is possible that the table data was created without content. 
     * Now that data is going to be written to the table, initialize */ 
    if (column_count) {
        columns = data->columns;

        if (!columns) {
            ecs_init_data(world, table, data);
            columns = data->columns;
        }

        *column_count_out = column_count;
        *columns_out = columns;
    }
}

static
void grow_column(
    ecs_column_t * column,
    ecs_lifecycle_t * lc,
    int32_t to_add,
    int32_t new_size,
    bool construct)
{
    ecs_vector_t *vec = column->data;
    int16_t alignment = column->alignment;

    int32_t size = column->size;
    int32_t count = ecs_vector_count(vec);
    int32_t old_size = ecs_vector_size(vec);
    int32_t new_count = count + to_add;
    bool can_realloc = new_size != old_size;

    ecs_assert(new_size >= new_count, ECS_INTERNAL_ERROR);

    /* If the array could possibly realloc and the component has a move action 
     * defined, move old elements manually */
    ecs_move_t move;
    if (lc && count && can_realloc && (move = lc->move)) {
        ecs_xtor_t ctor = lc->ctor;
        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);

        /* Create new vector */
        ecs_vector_t *new_vec = ecs_vector_new_t(size, alignment, new_size);
        ecs_vector_set_count_t(&new_vec, size, alignment, new_count);

        void *old_buffer = ecs_vector_first_t(
            vec, size, alignment);

        void *new_buffer = ecs_vector_first_t(
            new_vec, size, alignment);

        /* First construct elements (old and new) in new buffer */
        ctor(new_buffer, ecs_to_size_t(size), construct ? new_count : count);
        
        /* Move old elements */
        move(new_buffer, old_buffer, ecs_to_size_t(size), count);

        /* Free old vector */
        ecs_vector_free(vec);
        column->data = new_vec;
    } else {
        /* If array won't realloc or has no move, simply add new elements */
        if (can_realloc) {
            ecs_vector_set_size_t(&vec, size, alignment, new_size);
        }

        void *elem = ecs_vector_addn_t(&vec, size, alignment, to_add);

        ecs_xtor_t ctor;
        if (construct && lc && (ctor = lc->ctor)) {
            /* If new elements need to be constructed and component has a
             * constructor, construct */
            ctor(elem, ecs_to_size_t(size), to_add);
        }

        column->data = vec;
    }

    ecs_assert(ecs_vector_size(column->data) == new_size, 
        ECS_INTERNAL_ERROR);
}

static
int32_t grow_data(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t to_add,
    int32_t size,
    const ecs_entity_t *ids)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    int32_t cur_count = ecs_table_data_count(data);
    int32_t column_count = table->column_count;
    ecs_column_t *columns;
    ensure_data(world, table, data, &column_count, &columns);

    /* Add record to record ptr array */
    ecs_vector_set_size(&data->record_ptrs, ecs_record_t*, size);
    ecs_record_t **r = ecs_vector_addn(&data->record_ptrs, ecs_record_t*, to_add);
    ecs_assert(r != NULL, ECS_INTERNAL_ERROR);
    if (ecs_vector_size(data->record_ptrs) > size) {
        size = ecs_vector_size(data->record_ptrs);
    }

    /* Add entity to column with entity ids */
    ecs_vector_set_size(&data->entities, ecs_entity_t, size);
    ecs_entity_t *e = ecs_vector_addn(&data->entities, ecs_entity_t, to_add);
    ecs_assert(e != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_vector_size(data->entities) == size, ECS_INTERNAL_ERROR);

    /* Initialize entity ids and record ptrs */
    int32_t i;
    if (ids) {
        for (i = 0; i < to_add; i ++) {
            e[i] = ids[i];
        }
    } else {
        ecs_os_memset(e, 0, ECS_SIZEOF(ecs_entity_t) * to_add);
    }
    ecs_os_memset(r, 0, ECS_SIZEOF(ecs_record_t*) * to_add);

    /* Add elements to each column array */
    ecs_lifecycle_t *lc_array = data->lc;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        if (!column->size) {
            continue;
        }

        ecs_lifecycle_t *lc = NULL;
        if (lc_array) {
            lc = &lc_array[i];
        }

        grow_column(column, lc, to_add, size, true);
        ecs_assert(ecs_vector_size(columns[i].data) == size, 
            ECS_INTERNAL_ERROR);
    }

    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    if (!world->in_progress && !cur_count) {
        // ecs_table_activate(world, table, 0, true);
    }

    data->alloc_count ++;

    /* Return index of first added entity */
    return cur_count;
}

static
void fast_append(
    ecs_column_t *columns,
    int32_t column_count)
{
    /* Add elements to each column array */
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        if (size) {
            int16_t alignment = column->alignment;
            ecs_vector_add_t(&column->data, size, alignment);
        }
    }
}

int32_t ecs_table_append(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    ecs_entity_t entity,
    ecs_record_t * record,
    bool construct)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    /* Get count & size before growing entities array. This tells us whether the
     * arrays will realloc */
    int32_t count = ecs_vector_count(data->entities);
    int32_t size = ecs_vector_size(data->entities);

    int32_t column_count = table->column_count;
    ecs_column_t *columns;
    ensure_data(world, table, data, &column_count, &columns);

    /* Grow buffer with entity ids, set new element to new entity */
    ecs_entity_t *e = ecs_vector_add(&data->entities, ecs_entity_t);
    ecs_assert(e != NULL, ECS_INTERNAL_ERROR);
    *e = entity;    

    /* Keep track of alloc count. This allows references to check if cached
     * pointers need to be updated. */  
    data->alloc_count += (count == size);

    /* Add record ptr to array with record ptrs */
    ecs_record_t **r = ecs_vector_add(&data->record_ptrs, ecs_record_t*);
    ecs_assert(r != NULL, ECS_INTERNAL_ERROR);
    *r = record;
 
    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    /* If this is the first entity in this table, signal queries so that the
     * table moves from an inactive table to an active table. */
    if (!world->in_progress && !count) {
        // ecs_table_activate(world, table, 0, true);
    }

    ecs_assert(count >= 0, ECS_INTERNAL_ERROR);

    /* Fast path: no switch columns, no lifecycle actions */
    if (!(table->flags & EcsTableIsComplex)) {
        fast_append(columns, column_count);
        return count;
    }

    ecs_lifecycle_t *lc_array = data->lc;

    /* Reobtain size to ensure that the columns have the same size as the 
     * entities and record vectors. This keeps reasoning about when allocations
     * occur easier. */
    size = ecs_vector_size(data->entities);

    /* Grow component arrays with 1 element */
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        if (!column->size) {
            continue;
        }

        ecs_lifecycle_t *lc = NULL;
        if (lc_array) {
            lc = &lc_array[i];
        }

        grow_column(column, lc, 1, size, construct);
        
        ecs_assert(
            ecs_vector_size(columns[i].data) == ecs_vector_size(data->entities), 
            ECS_INTERNAL_ERROR); 
            
        ecs_assert(
            ecs_vector_count(columns[i].data) == ecs_vector_count(data->entities), 
            ECS_INTERNAL_ERROR);                        
    }

    return count;
}

static
void fast_delete_last(
    ecs_column_t *columns,
    int32_t column_count) 
{
    int i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        ecs_vector_remove_last(column->data);
    }
}

static
void fast_delete(
    ecs_column_t *columns,
    int32_t column_count,
    int32_t index) 
{
    int i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        if (size) {
            int16_t alignment = column->alignment;
            ecs_vector_remove_t(column->data, size, alignment, index);
        } 
    }
}

void ecs_table_delete(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t index,
    bool destruct)
{
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_vector_t *entity_column = data->entities;
    int32_t count = ecs_vector_count(entity_column);

    ecs_assert(count > 0, ECS_INTERNAL_ERROR);
    count --;
    
    ecs_assert(index <= count, ECS_INTERNAL_ERROR);

    ecs_lifecycle_t *lc_array = data->lc;
    int32_t column_count = table->column_count;
    int32_t i;

    ecs_entity_t *entities = ecs_vector_first(entity_column, ecs_entity_t);
    ecs_entity_t entity_to_move = entities[count];

    /* Move last entity id to index */
    entities[index] = entity_to_move;
    ecs_vector_remove_last(entity_column);

    /* Move last record ptr to index */
    ecs_vector_t *record_column = data->record_ptrs;     
    ecs_record_t **records = ecs_vector_first(record_column, ecs_record_t*);

    ecs_assert(count < ecs_vector_count(record_column), ECS_INTERNAL_ERROR);
    ecs_record_t *record_to_move = records[count];

    records[index] = record_to_move;
    ecs_vector_remove_last(record_column);    

    /* Update record of moved entity in entity index */
    if (index != count) {
        if (record_to_move) {
            if (record_to_move->row >= 0) {
                record_to_move->row = index + 1;
            } else {
                record_to_move->row = -(index + 1);
            }
            ecs_assert(record_to_move->table != NULL, ECS_INTERNAL_ERROR);
            ecs_assert(record_to_move->table == table, ECS_INTERNAL_ERROR);
        }
    } 

    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    if (!count) {
        // ecs_table_activate(world, table, NULL, false);
    }

    /* Move each component value in array to index */
    ecs_column_t *columns = data->columns;

    if (!(table->flags & EcsTableIsComplex)) {
        if (index == count) {
            fast_delete_last(columns, column_count);
        } else {
            fast_delete(columns, column_count, index);
        }
        return;
    }

    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        int16_t alignment = column->alignment;
        if (size) {
            ecs_lifecycle_t *lc = lc_array ? &lc_array[i] : NULL;
            ecs_xtor_t dtor;

            void *dst = ecs_vector_get_t(column->data, size, alignment, index);

            ecs_move_t move;
            if (lc && (count != index) && (move = lc->move)) {
                void *src = ecs_vector_get_t(column->data, size, alignment, count);

                /* If the delete is not destructing the component, the component
                * was already deleted, most likely by a move. In that case we
                * still need to move, but we need to make sure we're moving
                * into an element that is initialized with valid memory, so
                * call the constructor. */
                if (!destruct) {
                    ecs_xtor_t ctor = lc->ctor;
                    ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);
                    ctor(dst, ecs_to_size_t(size), 1);   
                }

                /* Move last element into deleted element */
                move(dst, src, ecs_to_size_t(size), 1);

                /* Memory has been copied, we can now simply remove last */
                ecs_vector_remove_last(column->data);                              
            } else {
                if (destruct && lc && (dtor = lc->dtor)) {
                    dtor(dst, ecs_to_size_t(size), 1);
                }

                ecs_vector_remove_t(column->data, size, alignment, index);
            }
        }
    }
}

static
void fast_move(
    ecs_table_t * new_table,
    ecs_data_t * new_data,
    int32_t new_index,
    ecs_table_t * old_table,
    ecs_data_t * old_data,
    int32_t old_index)
{
    ecs_type_t new_type = new_table->type;
    ecs_type_t old_type = old_table->type;

    int32_t i_new = 0, new_column_count = new_table->column_count;
    int32_t i_old = 0, old_column_count = old_table->column_count;
    ecs_entity_t *new_components = ecs_vector_first(new_type, ecs_entity_t);
    ecs_entity_t *old_components = ecs_vector_first(old_type, ecs_entity_t);

    ecs_column_t *old_columns = old_data->columns;
    ecs_column_t *new_columns = new_data->columns;

    for (; (i_new < new_column_count) && (i_old < old_column_count);) {
        ecs_entity_t new_component = new_components[i_new];
        ecs_entity_t old_component = old_components[i_old];

        if (new_component == old_component) {
            ecs_column_t *new_column = &new_columns[i_new];
            ecs_column_t *old_column = &old_columns[i_old];
            int16_t size = new_column->size;

            if (size) {
                int16_t alignment = new_column->alignment;
                void *dst = ecs_vector_get_t(new_column->data, size, alignment, new_index);
                void *src = ecs_vector_get_t(old_column->data, size, alignment, old_index);

                ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);
                ecs_assert(src != NULL, ECS_INTERNAL_ERROR);
                ecs_os_memcpy(dst, src, size); 
            }
        }

        i_new += new_component <= old_component;
        i_old += new_component >= old_component;
    }
}

void ecs_table_move(
    ecs_world_t * world,
    ecs_entity_t dst_entity,
    ecs_entity_t src_entity,
    ecs_table_t *new_table,
    ecs_data_t *new_data,
    int32_t new_index,
    ecs_table_t *old_table,
    ecs_data_t *old_data,
    int32_t old_index)
{
    (void)world;
    
    ecs_assert(new_table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(old_table != NULL, ECS_INTERNAL_ERROR);

    ecs_assert(old_index >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(new_index >= 0, ECS_INTERNAL_ERROR);

    ecs_assert(old_data != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(new_data != NULL, ECS_INTERNAL_ERROR);

    if (!((new_table->flags | old_table->flags) & EcsTableIsComplex)) {
        fast_move(new_table, new_data, new_index, old_table, old_data, old_index);
        return;
    }

    bool same_entity = dst_entity == src_entity;

    ecs_type_t new_type = new_table->type;
    ecs_type_t old_type = old_table->type;

    int32_t i_new = 0, new_column_count = new_table->column_count;
    int32_t i_old = 0, old_column_count = old_table->column_count;
    ecs_entity_t *new_components = ecs_vector_first(new_type, ecs_entity_t);
    ecs_entity_t *old_components = ecs_vector_first(old_type, ecs_entity_t);

    ecs_column_t *old_columns = old_data->columns;
    ecs_column_t *new_columns = new_data->columns;

    for (; (i_new < new_column_count) && (i_old < old_column_count);) {
        ecs_entity_t new_component = new_components[i_new];
        ecs_entity_t old_component = old_components[i_old];

        if (new_component == old_component) {
            ecs_column_t *new_column = &new_columns[i_new];
            ecs_column_t *old_column = &old_columns[i_old];
            int16_t size = new_column->size;
            int16_t alignment = new_column->alignment;

            if (size) {
                void *dst = ecs_vector_get_t(new_column->data, size, alignment, new_index);
                void *src = ecs_vector_get_t(old_column->data, size, alignment, old_index);

                ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);
                ecs_assert(src != NULL, ECS_INTERNAL_ERROR);

                ecs_lifecycle_t *lc = &new_data->lc[i_new];
                if (same_entity) {
                    ecs_move_t move;
                    if (lc && (move = lc->move)) {
                        ecs_xtor_t ctor = lc->ctor;

                        /* Ctor should always be set if copy is set */
                        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);

                        /* Construct a new value, move the value to it */
                        ctor(dst, ecs_to_size_t(size), 1);
                        move(dst, src, ecs_to_size_t(size), 1);
                    } else {
                        ecs_os_memcpy(dst, src, size);
                    }
                } else {
                    ecs_copy_t copy;
                    if (lc && (copy = lc->copy)) {
                        ecs_xtor_t ctor = lc->ctor;

                        /* Ctor should always be set if copy is set */
                        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);
                        ctor(dst, ecs_to_size_t(size), 1);
                        copy(dst, src, ecs_to_size_t(size), 1);
                    } else {
                        ecs_os_memcpy(dst, src, size);
                    }
                }
            }
        } else {
            if (new_component < old_component) {
                ctor_component(&new_data->lc[i_new],
                    &new_columns[i_new], new_index, 1);
            } else {
                dtor_component(&old_data->lc[i_old],
                    &old_columns[i_old], old_index, 1);
            }
        }

        i_new += new_component <= old_component;
        i_old += new_component >= old_component;
    }

    for (; (i_new < new_column_count); i_new ++) {
        ctor_component(&new_data->lc[i_new], &new_columns[i_new], new_index, 1);
    }

    for (; (i_old < old_column_count); i_old ++) {
        dtor_component(&old_data->lc[i_old], &old_columns[i_old], old_index, 1);
    }
}

int32_t ecs_table_appendn(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t to_add,
    const ecs_entity_t *ids)
{
    int32_t cur_count = ecs_table_data_count(data);
    return grow_data(world, table, data, to_add, cur_count + to_add, ids);
}

void ecs_table_set_size(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t size)
{
    int32_t cur_count = ecs_table_data_count(data);

    if (cur_count < size) {
        grow_data(world, table, data, 0, size, NULL);
    } else if (!size) {
        /* Initialize columns if 0 is passed. This is a shortcut to initialize
         * columns when, for example, an API call is inserting bulk data. */
        int32_t column_count = table->column_count;
        ecs_column_t *columns;
        ensure_data(world, table, data, &column_count, &columns);
    }
}

int32_t ecs_table_data_count(
    ecs_data_t *data)
{
    return data ? ecs_vector_count(data->entities) : 0;
}

int32_t ecs_table_count(
    ecs_table_t *table)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_data_t *data = table->data;
    if (!data) {
        return 0;
    }

    return ecs_table_data_count(data);
}

int32_t* ecs_table_get_dirty_state(
    ecs_table_t *table)
{
    ecs_data_t *data = ecs_table_get_or_create_data(table);
    if (!data->dirty_state) {
        data->dirty_state = ecs_os_calloc(ECS_SIZEOF(int32_t) * (table->column_count + 1));
        ecs_assert(data->dirty_state != NULL, ECS_INTERNAL_ERROR);
    }
    return data->dirty_state;
}

int32_t* ecs_table_get_monitor(
    ecs_table_t *table)
{
    int32_t *dirty_state = ecs_table_get_dirty_state(table);
    ecs_assert(dirty_state != NULL, ECS_INTERNAL_ERROR);

    int32_t column_count = table->column_count;
    return ecs_os_memdup(dirty_state, (column_count + 1) * ECS_SIZEOF(int32_t));
}

void ecs_table_notify(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_table_event_t * event)
{
    switch(event->kind) {
    case EcsTableComponentInfo:
        notify_component_info(world, table, event->component);
        break;
    }
}
