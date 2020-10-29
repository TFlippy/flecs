#ifndef FLECS_DENSE_H
#define FLECS_DENSE_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecs_dense_t ecs_dense_t;

FLECS_API ecs_dense_t* _ecs_dense_new(
    ecs_size_t dense_size,
    ecs_size_t dense_alignment);

FLECS_API void* _ecs_dense_get(
    const ecs_dense_t *dense,
    ecs_size_t size,
    uint64_t id);

FLECS_API void* _ecs_dense_ensure(
    ecs_dense_t *dense,
    ecs_size_t size,
    uint64_t id);

FLECS_API void ecs_dense_remove(
    ecs_dense_t *dense,
    uint64_t id);    

FLECS_API void ecs_dense_free(
    ecs_dense_t *dense);

#define ecs_dense_new(T)\
    _ecs_dense_new(sizeof(T), ECS_ALIGNOF(T))

#define ecs_dense_get(dense, T, id)\
    _ecs_dense_get(dense, sizeof(T), id)

#define ecs_dense_ensure(dense, T, id)\
    _ecs_dense_ensure(dense, sizeof(T), id)

#ifdef __cplusplus
}
#endif

#endif
