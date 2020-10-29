#ifndef FLECS_SPARSE_H
#define FLECS_SPARSE_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecs_sparse_t ecs_sparse_t;

FLECS_API ecs_sparse_t* _ecs_sparse_new(
    ecs_size_t elem_size);

FLECS_API void ecs_sparse_set_id_source(
    ecs_sparse_t *sparse,
    uint64_t *id_source);

FLECS_API void ecs_sparse_free(
    ecs_sparse_t *sparse);

FLECS_API void ecs_sparse_clear(
    ecs_sparse_t *sparse);

FLECS_API void* _ecs_sparse_add(
    ecs_sparse_t *sparse,
    ecs_size_t elem_size);

FLECS_API uint64_t ecs_sparse_last_id(
    ecs_sparse_t *sparse);

FLECS_API uint64_t ecs_sparse_new_id(
    ecs_sparse_t *sparse);

FLECS_API const uint64_t* ecs_sparse_new_ids(
    ecs_sparse_t *sparse,
    int32_t count);

FLECS_API void ecs_sparse_remove(
    ecs_sparse_t *sparse,
    uint64_t id);

FLECS_API void* _ecs_sparse_remove_get(
    ecs_sparse_t *sparse,
    ecs_size_t elem_size,
    uint64_t id);    

FLECS_API void ecs_sparse_set_generation(
    ecs_sparse_t *sparse,
    uint64_t id);    

FLECS_API bool ecs_sparse_exists(
    ecs_sparse_t *sparse,
    uint64_t id);

FLECS_API void* _ecs_sparse_get_elem(
    const ecs_sparse_t *sparse,
    ecs_size_t elem_size,
    int32_t elem);

FLECS_API bool ecs_sparse_is_alive(
    const ecs_sparse_t *sparse,
    uint64_t id);

FLECS_API int32_t ecs_sparse_count(
    const ecs_sparse_t *sparse);

FLECS_API int32_t ecs_sparse_size(
    const ecs_sparse_t *sparse);

FLECS_API void* _ecs_sparse_get(
    const ecs_sparse_t *sparse,
    ecs_size_t elem_size,
    uint64_t id);

FLECS_API void* _ecs_sparse_get_any(
    ecs_sparse_t *sparse,
    ecs_size_t elem_size,
    uint64_t id);

FLECS_API void* _ecs_sparse_get_or_create(
    ecs_sparse_t *sparse,
    ecs_size_t elem_size,
    uint64_t id);

FLECS_API const uint64_t* ecs_sparse_ids(
    const ecs_sparse_t *sparse);

FLECS_API void ecs_sparse_set_size(
    ecs_sparse_t *sparse,
    int32_t elem_count);

FLECS_API ecs_sparse_t* ecs_sparse_copy(
    const ecs_sparse_t *src);    

FLECS_API void ecs_sparse_restore(
    ecs_sparse_t *dst,
    const ecs_sparse_t *src);

FLECS_API void ecs_sparse_memory(
    const ecs_sparse_t *sparse,
    int32_t *allocd,
    int32_t *used);

#define ecs_sparse_new(type)\
    _ecs_sparse_new(sizeof(type))

#define ecs_sparse_add(sparse, type)\
    ((type*)_ecs_sparse_add(sparse, sizeof(type)))

#define ecs_sparse_remove_get(sparse, type, id)\
    ((type*)_ecs_sparse_remove_get(sparse, sizeof(type), id))

#define ecs_sparse_get_elem(sparse, type, id)\
    ((type*)_ecs_sparse_get_elem(sparse, sizeof(type), id))

#define ecs_sparse_get(sparse, type, id)\
    ((type*)_ecs_sparse_get(sparse, sizeof(type), id))

#define ecs_sparse_get_any(sparse, type, id)\
    ((type*)_ecs_sparse_get_any(sparse, sizeof(type), id))

#define ecs_sparse_get_or_create(sparse, type, id)\
    ((type*)_ecs_sparse_get_or_create(sparse, sizeof(type), id))

#ifdef __cplusplus
}
#endif

#endif
