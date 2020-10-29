/**
 * @file type.h
 * @brief Type API.
 */

#ifndef FLECS_TYPE_H
#define FLECS_TYPE_H

typedef const ecs_vector_t* ecs_type_t;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_API char* ecs_type_str(
    ecs_world_t *world,
    ecs_type_t type);    

FLECS_API bool ecs_type_has_entity(
    const ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t entity);

FLECS_API int32_t ecs_type_index_of(
    ecs_type_t type,
    ecs_entity_t component);

FLECS_API ecs_entity_t ecs_type_contains(
    ecs_world_t *world,
    ecs_type_t type_id_1,
    ecs_type_t type_id_2,
    bool match_all);

#ifdef __cplusplus
}
#endif

#endif
