
#ifndef FLECS_H
#define FLECS_H

// #define FLECS_LEGACY         /* Define for C89 support */
// #define FLECS_CUSTOM_BUILD   /* Define for customizing a build */

#include "flecs/private/api_defines.h"

typedef uint64_t ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;

#include "flecs/private/vector.h"
#include "flecs/private/paged.h"
#include "flecs/private/sparse.h"
#include "flecs/private/dense.h"
#include "flecs/private/ptree.h"
#include "flecs/private/map.h"
#include "flecs/os_api.h"
#include "flecs/private/api_types.h"
#include "flecs/private/log.h"
#include "flecs/type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EcsName {
    const char *value;
    const char *symbol;
    char *alloc_value;
} EcsName;

typedef struct EcsComponent {
    ecs_size_t size;
    ecs_size_t alignment;
} EcsComponent;

typedef struct ecs_lifecycle_t {
    ecs_xtor_t ctor;
    ecs_xtor_t dtor;
    ecs_copy_t copy;
    ecs_move_t move;
    bool is_set;
} ecs_lifecycle_t;


/**
 * @defgroup world_api World API
 * @{
 */

FLECS_API ecs_world_t* ecs_init(void);

FLECS_API int ecs_fini(
    ecs_world_t *world);

FLECS_API void ecs_set_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component,
    ecs_lifecycle_t *actions);

FLECS_API void ecs_dim(
    ecs_world_t *world,
    int32_t entity_count);

FLECS_API void ecs_defer_begin(
    ecs_world_t *world);

FLECS_API void ecs_defer_end(
    ecs_world_t *world);

FLECS_API void ecs_tracing_enable(
    int level);

FLECS_API void ecs_make_sparse(
    ecs_world_t *world,
    ecs_entity_t id);

/** @} */


/**
 * @defgroup basic Basic ECS operations
 * @{
 */

FLECS_API ecs_entity_t ecs_new_id(
    ecs_world_t *world);

FLECS_API ecs_entity_t ecs_new_component_id(
    ecs_world_t *world);

FLECS_API void ecs_add_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id);

FLECS_API void ecs_remove_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id);

FLECS_API void ecs_clear(
    ecs_world_t *world,
    ecs_entity_t entity);

FLECS_API void ecs_delete(
    ecs_world_t *world,
    ecs_entity_t entity);

FLECS_API const void* ecs_get_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id);

FLECS_API void* ecs_get_mut_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id,
    bool *is_added);

FLECS_API void ecs_modified_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id);

FLECS_API ecs_entity_t ecs_set_ptr_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id,
    const void *ptr,
    size_t size);

FLECS_API bool ecs_has_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t id);

FLECS_API bool ecs_is_alive(
    const ecs_world_t *world,
    ecs_entity_t entity);

FLECS_API bool ecs_exists(
    const ecs_world_t *world,
    ecs_entity_t entity);

FLECS_API ecs_type_t ecs_get_type(
    const ecs_world_t *world,
    ecs_entity_t entity);

FLECS_API const char* ecs_get_name(
    const ecs_world_t *world,
    ecs_entity_t entity);

/** @} */


/**
 * @defgroup scopes Scoping
 * @{
 */

FLECS_API ecs_entity_t ecs_set_scope(
    ecs_world_t *world,
    ecs_entity_t scope);

FLECS_API ecs_entity_t ecs_get_scope(
    const ecs_world_t *world);

/** @} */


/**
 * @defgroup macro_api Macro wrappers for type safety & convenience
 * @{
 */

#define ECS_ROLE ((uint64_t)1 << 63)

#define ecs_role(role, id) (ECS_ROLE | ((uint64_t)role << 32) | (uint32_t)id)

#define ecs_get_role(id) ((id & ~ECS_ROLE) >> 32)

#define ecs_has_role(role, id) (ecs_get_role(id) == role)

#define ecs_typeid(T) FLECS__E##T


#ifndef FLECS_LEGACY

#define ecs_has(world, entity, type)\
    (ecs_has_id(world, entity, ecs_typeid(type)))

#define ecs_get(world, entity, type)\
    ((const type*)ecs_get_id(world, entity, ecs_typeid(type)))

#define ecs_get_mut(world, entity, type, is_added)\
    ((type*)ecs_get_mut_id(world, entity, ecs_typeid(type), is_added))

#define ecs_set(world, entity, type, ...)\
    (ecs_set_ptr_id(world, entity, ecs_typeid(type), &(type)__VA_ARGS__, sizeof(type)))

#endif

/** @} */ 


#ifdef __cplusplus
}
#endif

#endif
