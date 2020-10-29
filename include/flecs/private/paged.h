#ifndef FLECS_PAGED_H
#define FLECS_PAGED_H

#include "api_defines.h"

typedef struct ecs_paged_t {
    ecs_vector_t *pages;
    ecs_size_t *column_sizes;
    int32_t column_count;
    int32_t count;
} ecs_paged_t;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_API ecs_paged_t* ecs_paged_new(
    int32_t column_count,
    ecs_size_t *column_sizes);

FLECS_API void ecs_paged_init(
    ecs_paged_t *paged,
    int32_t column_count,
    ecs_size_t *column_sizes);

FLECS_API void ecs_paged_deinit(
    ecs_paged_t *paged);    

FLECS_API void ecs_paged_clear(
    ecs_paged_t *paged);

FLECS_API void ecs_paged_free(
    ecs_paged_t *paged);

FLECS_API void* _ecs_paged_get(
    const ecs_paged_t *paged,
    ecs_size_t size,
    int32_t index,
    int32_t column);

FLECS_API void* _ecs_paged_ensure(
    ecs_paged_t *paged,
    ecs_size_t size,
    int32_t index,
    int32_t column);

FLECS_API int32_t ecs_paged_count(
    const ecs_paged_t *paged);

#define ecs_paged_get(paged, T, index, column)\
    ((T*)_ecs_paged_get(paged, ECS_SIZEOF(T), index, column))

#define ecs_paged_ensure(paged, T, index, column)\
    ((T*)_ecs_paged_ensure(paged, ECS_SIZEOF(T), index, column))

#ifdef __cplusplus
}
#endif

#endif
