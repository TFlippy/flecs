#include "private_api.h"

static
ecs_defer_t* new_defer_op(ecs_stage_t *stage) {
    ecs_defer_t *result = ecs_vector_add(&stage->defer_queue, ecs_defer_t);
    ecs_os_memset(result, 0, ECS_SIZEOF(ecs_defer_t));
    return result;
}

static 
void new_defer_component_ids(
    ecs_defer_t *op, 
    ecs_entities_t *components)
{
    int32_t components_count = components->count;
    if (components_count == 1) {
        ecs_entity_t component = components->array[0];
        op->component = component;
        op->components = (ecs_entities_t) {
            .array = NULL,
            .count = 1
        };
    } else if (components_count) {
        ecs_size_t array_size = components_count * ECS_SIZEOF(ecs_entity_t);
        op->components.array = ecs_os_malloc(array_size);
        ecs_os_memcpy(op->components.array, components->array, array_size);
        op->components.count = components_count;
    } else {
        op->component = 0;
        op->components = (ecs_entities_t){ 0 };
    }
}

static
bool defer_new(
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entities_t *components)
{
    if (stage->defer) {
        ecs_entity_t scope = stage->scope;
        if (components) {
            if (!components->count && !scope) {
                return true;
            }
        }

        ecs_defer_t *op = new_defer_op(stage);
        op->kind = op_kind;
        op->scope = scope;
        op->is._1.entity = entity;

        new_defer_component_ids(op, components);

        return true;
    } else {
        stage->defer ++;
    }
    
    return false;
}

static
bool defer_add_remove(
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    if (stage->defer) {
        ecs_entity_t scope = stage->scope;

        ecs_defer_t *op = new_defer_op(stage);
        op->kind = op_kind;
        op->scope = scope;
        op->is._1.entity = entity;
        op->component = component;

        return true;
    } else {
        stage->defer ++;
    }
    
    return false;
}

bool ecs_defer_none(
    ecs_world_t *world,
    ecs_stage_t *stage)
{
    (void)world;
    stage->defer ++;
    return false;
}

bool ecs_defer_modified(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    (void)world;
    if (stage->defer) {
        ecs_defer_t *op = new_defer_op(stage);
        op->kind = EcsOpModified;
        op->component = component;
        op->is._1.entity = entity;
        return true;
    } else {
        stage->defer ++;
    }
    
    return false;
}

bool ecs_defer_delete(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity)
{
    (void)world;
    if (stage->defer) {
        ecs_defer_t *op = new_defer_op(stage);
        op->kind = EcsOpDelete;
        op->is._1.entity = entity;
        return true;
    } else {
        stage->defer ++;
    }
    return false;
}

bool ecs_defer_clear(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity)
{
    (void)world;
    if (stage->defer) {
        ecs_defer_t *op = new_defer_op(stage);
        op->kind = EcsOpClear;
        op->is._1.entity = entity;
        return true;
    } else {
        stage->defer ++;
    }
    return false;
}

bool ecs_defer_new(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entities_t *components)
{   
    (void)world;
    return defer_new(stage, EcsOpNew, entity, components);
}

bool ecs_defer_add(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component)
{   
    (void)world;
    return defer_add_remove(stage, EcsOpAdd, entity, component);
}

bool ecs_defer_remove(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    (void)world;
    return defer_add_remove(stage, EcsOpRemove, entity, component);
}

bool ecs_defer_set(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entity_t component,
    ecs_size_t size,
    const void *value,
    void **value_out,
    bool *is_added)
{
    if (stage->defer) {
        if (!size) {
            const EcsComponent *cptr = ecs_get(world, component, EcsComponent);
            ecs_assert(cptr != NULL, ECS_INVALID_PARAMETER);
            size = cptr->size;
        }

        ecs_defer_t *op = new_defer_op(stage);
        op->kind = op_kind;
        op->component = component;
        op->is._1.entity = entity;
        op->is._1.size = size;
        op->is._1.value = ecs_os_malloc(size);

        if (!value) {
            value = ecs_get_id(world, entity, component);
            if (is_added) {
                *is_added = value == NULL;
            }
        }

        ecs_lifecycle_t *lc = NULL;
        ecs_entity_t real_id = ecs_component_id_from_id(world, component);
        if (real_id) {
            lc = ecs_get_lifecycle(world, real_id);
        }
        ecs_xtor_t ctor;
        if (lc && (ctor = lc->ctor)) {
            ctor(op->is._1.value, ecs_to_size_t(size), 1);

            ecs_copy_t copy;
            if (value) {
                if ((copy = lc->copy)) {
                    copy(op->is._1.value, value, ecs_to_size_t(size), 1);
                } else {
                    ecs_os_memcpy(op->is._1.value, value, size);
                }
            }
        } else if (value) {
            ecs_os_memcpy(op->is._1.value, value, size);
        }
        
        if (value_out) {
            *value_out = op->is._1.value;
        }

        return true;
    } else {
        stage->defer ++;
    }
    
    return false;
}

void ecs_stage_merge(
    ecs_world_t *world,
    ecs_stage_t *stage)
{
    ecs_assert(stage != &world->stage, ECS_INTERNAL_ERROR);

    ecs_assert(stage->defer == 0, ECS_INVALID_PARAMETER);
    if (ecs_vector_count(stage->defer_merge_queue)) {
        stage->defer ++;
        stage->defer_queue = stage->defer_merge_queue;
        ecs_defer_flush(world, stage);
        ecs_vector_clear(stage->defer_merge_queue);
        ecs_assert(stage->defer_queue == NULL, ECS_INVALID_PARAMETER);
    }    
}

void ecs_stage_defer_begin(
    ecs_world_t *world,
    ecs_stage_t *stage)
{   
    (void)world; 
    ecs_defer_none(world, stage);
    if (stage->defer == 1) {
        stage->defer_queue = stage->defer_merge_queue;
    }
}

void ecs_stage_defer_end(
    ecs_world_t *world,
    ecs_stage_t *stage)
{ 
    (void)world;
    stage->defer --;
    if (!stage->defer) {
        stage->defer_merge_queue = stage->defer_queue;
        stage->defer_queue = NULL;
    }
}

void ecs_stage_init(
    ecs_world_t *world,
    ecs_stage_t *stage)
{
    memset(stage, 0, sizeof(ecs_stage_t));
    if (stage == &world->stage) {
        stage->id = 0;
    } else if (stage == &world->temp_stage) {
        stage->id = 1;
    }
}

void ecs_stage_deinit(
    ecs_world_t *world,
    ecs_stage_t *stage)
{
    (void)world;
    ecs_vector_free(stage->defer_queue);
    ecs_vector_free(stage->defer_merge_queue);
}

