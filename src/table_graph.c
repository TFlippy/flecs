#include "private_api.h"

ecs_entity_t ecs_component_id_from_id(
    ecs_world_t *world,
    ecs_entity_t e)
{
    ecs_entity_t role = ecs_get_role(e);
    if (role) {
        if (ecs_has(world, role, EcsComponent)) {
            return role;
        }

        e &= ECS_ENTITY_MASK;
    }

    if (ecs_has(world, e, EcsComponent)) {
        return e;
    }

    return 0;
}

const EcsComponent* ecs_component_from_id(
    ecs_world_t *world,
    ecs_entity_t e)
{
    ecs_entity_t role = ecs_get_role(e);

    const EcsComponent *component = NULL;
    if (role) {
        component = ecs_get(world, role, EcsComponent);
        e &= ECS_ENTITY_MASK;
    }

    if (!component) {
        component = ecs_get(world, e, EcsComponent);
    }

    return component;
}

/* Count number of columns with data (excluding tags) */
static
int32_t data_column_count(
    ecs_world_t * world,
    ecs_table_t * table)
{
    int32_t count = 0;
    ecs_vector_each(table->type, ecs_entity_t, c_ptr, {
        ecs_entity_t component = *c_ptr;

        /* Typically all components will be clustered together at the start of
         * the type as components are created from a separate id pool, and type
         * vectors are sorted. 
         * Explicitly check for EcsComponent and EcsName since the ecs_has check
         * doesn't work during bootstrap. */
        const EcsComponent *c = NULL;
        if ((component == ecs_typeid(EcsComponent)) || 
            (component == ecs_typeid(EcsName)) || 
            (c = ecs_component_from_id(world, component)) != NULL) 
        {
            if (!c || c->size) {
                count = c_ptr_i + 1;
            }
        }
    });

    return count;
}

static
ecs_type_t entities_to_type(
    ecs_entities_t *entities)
{
    if (entities->count) {
        ecs_vector_t *result = NULL;
        ecs_vector_set_count(&result, ecs_entity_t, entities->count);
        ecs_entity_t *array = ecs_vector_first(result, ecs_entity_t);
        ecs_os_memcpy(array, entities->array, ECS_SIZEOF(ecs_entity_t) * entities->count);
        return result;
    } else {
        return NULL;
    }
}

static
void init_edges(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_entity_t *entities = ecs_vector_first(table->type, ecs_entity_t);
    int32_t count = ecs_vector_count(table->type);

    table->edges = ecs_ptiny_new(ecs_edge_t);
    
    int32_t i;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = entities[i];

        ecs_edge_t *edge = ecs_ptiny_ensure(table->edges, ecs_edge_t, e);
        edge->add = table;

        /* As we're iterating over the table components, also set the table
         * flags. These allow us to quickly determine if the table contains
         * data that needs to be handled in a special way, like prefabs or 
         * containers */
        if (e <= EcsLastInternalComponentId) {
            table->flags |= EcsTableHasBuiltins;
        }

        if (e == EcsDisabled) {
            table->flags |= EcsTableIsDisabled;
        }

        if (e == ecs_typeid(EcsComponent)) {
            table->flags |= EcsTableHasComponentData;
        }
    }
}

static
void init_table(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_entities_t * entities)
{
    (void)world;

    table->type = entities_to_type(entities);
    table->column_count = data_column_count(world, table);

    uint64_t hash = 0;
    ecs_hash(entities->array, entities->count * ECS_SIZEOF(ecs_entity_t), &hash);
    ecs_map_set(world->store.table_map, hash, &table);

    init_edges(world, table);
}

static
ecs_table_t *create_table(
    ecs_world_t * world,
    ecs_entities_t * entities)
{
    ecs_table_t *result = ecs_sparse_add(world->store.tables, ecs_table_t);
    result->id = ecs_to_u32(ecs_sparse_last_id(world->store.tables));

    ecs_assert(result != NULL, ECS_INTERNAL_ERROR);
    init_table(world, result, entities);

    return result;
}

static
void remove_entity_from_type(
    ecs_type_t type,
    ecs_entity_t remove,
    ecs_entities_t *out)
{
    int32_t count = ecs_vector_count(type);
    ecs_entity_t *array = ecs_vector_first(type, ecs_entity_t);

    int32_t i, el = 0;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = array[i];
        if (e != remove) {
            out->array[el ++] = e;
            ecs_assert(el <= count, ECS_INTERNAL_ERROR);
        }
    }

    out->count = el;
}

static
void add_entity_to_type(
    ecs_type_t type,
    ecs_entity_t add,
    ecs_entities_t *out)
{
    int32_t count = ecs_vector_count(type);
    ecs_entity_t *array = ecs_vector_first(type, ecs_entity_t);    
    bool added = false;

    int32_t i, el = 0;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = array[i];

        if (e > add && !added) {
            out->array[el ++] = add;
            added = true;
        }
        
        out->array[el ++] = e;
        ecs_assert(el <= out->count, ECS_INTERNAL_ERROR);
    }

    if (!added) {
        out->array[el ++] = add;
    }

    out->count = el;
    ecs_assert(out->count != 0, ECS_INTERNAL_ERROR);
}

static
ecs_table_t *find_or_create_remove(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t remove)
{
    ecs_type_t type = node->type;
    int32_t count = ecs_vector_count(type);

    ecs_entities_t entities = {
        .array = ecs_os_alloca(ECS_SIZEOF(ecs_entity_t) * count),
        .count = count
    };

    remove_entity_from_type(type, remove, &entities);

    ecs_table_t *result = ecs_table_find_or_create(world, &entities);
    if (!result) {
        return NULL;
    }

    if (result != node) {
        ecs_edge_t *edge = ecs_ptiny_ensure(result->edges, ecs_edge_t, remove);
        if (!edge->add) {
            edge->add = node;
        }
    }

    return result;    
}

static
ecs_table_t *find_or_create_add(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t add)
{
    ecs_type_t type = node->type;
    int32_t count = ecs_vector_count(type);

    ecs_entities_t entities = {
        .array = ecs_os_alloca(ECS_SIZEOF(ecs_entity_t) * (count + 1)),
        .count = count + 1
    };

    add_entity_to_type(type, add, &entities);

    ecs_table_t *result = ecs_table_find_or_create(world, &entities);
    
    if (result != node) {
        ecs_edge_t *edge = ecs_ptiny_ensure(result->edges, ecs_edge_t, add);
        if (!edge->remove) {
            edge->remove = node;
        }
    }

    return result;
}

ecs_table_t* ecs_table_traverse_remove(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t e)    
{
    node = node ? node : &world->store.root;

    ecs_edge_t *edge = ecs_ptiny_get(node->edges, ecs_edge_t, e);
    if (!edge) {
        edge = ecs_ptiny_ensure(node->edges, ecs_edge_t, e);
    }

    ecs_table_t *next = edge->remove;

    if (!next) {
        next = find_or_create_remove(world, node, e);
        ecs_assert(next != NULL, ECS_INTERNAL_ERROR);
        edge->add = next;
    }

    return next;
}

ecs_table_t* ecs_table_traverse_add(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t e)    
{
    node = node ? node : &world->store.root;

    ecs_edge_t *edge = ecs_ptiny_get(node->edges, ecs_edge_t, e);
    if (!edge) {
        edge = ecs_ptiny_ensure(node->edges, ecs_edge_t, e);
    }
    ecs_table_t *next = edge->add;

    if (!next) {
        next = find_or_create_add(world, node, e);
        ecs_assert(next != NULL, ECS_INTERNAL_ERROR);
        edge->add = next;
    }

    return next;
}

static
int ecs_entity_compare(
    const void *e1,
    const void *e2)
{
    ecs_entity_t v1 = *(ecs_entity_t*)e1;
    ecs_entity_t v2 = *(ecs_entity_t*)e2;
    if (v1 < v2) {
        return -1;
    } else if (v1 > v2) {
        return 1;
    } else {
        return 0;
    }
}

static
bool ecs_entity_array_is_ordered(
    ecs_entities_t *entities)
{
    ecs_entity_t prev = 0;
    ecs_entity_t *array = entities->array;
    int32_t i, count = entities->count;

    for (i = 0; i < count; i ++) {
        if (!array[i] && !prev) {
            continue;
        }
        if (array[i] <= prev) {
            return false;
        }
        prev = array[i];
    }    

    return true;
}

static
int32_t ecs_entity_array_dedup(
    ecs_entity_t *array,
    int32_t count)
{
    int32_t j, k;
    ecs_entity_t prev = array[0];

    for (k = j = 1; k < count; j ++, k++) {
        ecs_entity_t e = array[k];
        if (e == prev) {
            k ++;
        }

        array[j] = e;
        prev = e;
    }

    return count - (k - j);
}

static
ecs_table_t *find_or_create(
    ecs_world_t * world,
    ecs_entities_t * entities)
{    
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);

    /* Make sure array is ordered and does not contain duplicates */
    int32_t type_count = entities->count;
    ecs_entity_t *ordered = NULL;

    if (!type_count) {
        return &world->store.root;
    }

    if (!ecs_entity_array_is_ordered(entities)) {
        ecs_size_t size = ECS_SIZEOF(ecs_entity_t) * type_count;
        ordered = ecs_os_alloca(size);
        ecs_os_memcpy(ordered, entities->array, size);
        qsort(ordered, (size_t)type_count, sizeof(ecs_entity_t), ecs_entity_compare);
        type_count = ecs_entity_array_dedup(ordered, type_count);
    } else {
        ordered = entities->array;
    }

    uint64_t hash = 0;
    ecs_hash(entities->array, entities->count * ECS_SIZEOF(ecs_entity_t), &hash);
    ecs_table_t *table = ecs_map_get_ptr(world->store.table_map, ecs_table_t*, hash);
    if (table) {
        return table;
    }

    ecs_entities_t ordered_entities = {
        .array = ordered,
        .count = type_count
    };

    /* If we get here, the table has not been found. It has to be created. */
    
    ecs_table_t *result = create_table(world, &ordered_entities);

    ecs_assert(ordered_entities.count == ecs_vector_count(result->type), 
        ECS_INTERNAL_ERROR);

    return result;
}

ecs_table_t* ecs_table_find_or_create(
    ecs_world_t * world,
    ecs_entities_t * components)
{
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(!world->in_progress, ECS_INTERNAL_ERROR);

    return find_or_create(world, components);
}

ecs_table_t* ecs_table_from_type(
    ecs_world_t *world,
    ecs_type_t type)
{
    ecs_entities_t components = ecs_type_to_entities(type);
    return ecs_table_find_or_create(
        world, &components);
}

void ecs_init_root_table(
    ecs_world_t *world,
    ecs_table_t *root)
{
    ecs_entities_t entities = {
        .array = NULL,
        .count = 0
    };

    init_table(world, root, &entities);
}

void ecs_table_clear_edges(
    ecs_table_t *table)
{
    (void)table;
}
