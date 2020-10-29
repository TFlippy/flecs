#ifndef FLECS_VECTOR_H
#define FLECS_VECTOR_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public, so we can do compile-time offset calculation */
struct ecs_vector_t {
    int32_t count;
    int32_t size;
    
#ifndef NDEBUG
    int64_t elem_size;
#endif
};

#define ECS_VECTOR_U(size, alignment) size, ECS_MAX(ECS_SIZEOF(ecs_vector_t), alignment)
#define ECS_VECTOR_T(T) ECS_VECTOR_U(ECS_SIZEOF(T), ECS_ALIGNOF(T))

typedef struct ecs_vector_t ecs_vector_t;

typedef int (*ecs_comparator_t)(
    const void* p1,
    const void *p2);

FLECS_API ecs_vector_t* _ecs_vector_new(
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_new(T, elem_count) \
    _ecs_vector_new(ECS_VECTOR_T(T), elem_count)

#define ecs_vector_new_t(size, alignment, elem_count) \
    _ecs_vector_new(ECS_VECTOR_U(size, alignment), elem_count)    

FLECS_API ecs_vector_t* _ecs_vector_from_array(
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count,
    void *array);

#define ecs_vector_from_array(T, elem_count, array)\
    _ecs_vector_from_array(ECS_VECTOR_T(T), elem_count, array)

FLECS_API void ecs_vector_free(
    ecs_vector_t *vector);

FLECS_API void ecs_vector_clear(
    ecs_vector_t *vector);

FLECS_API void ecs_vector_assert_size(
    ecs_vector_t* vector_inout,
    ecs_size_t elem_size);

FLECS_API void ecs_vector_assert_alignment(
    ecs_vector_t* vector,
    ecs_size_t elem_alignment);    

FLECS_API void* _ecs_vector_add(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_add(vector, T) \
    ((T*)_ecs_vector_add(vector, ECS_VECTOR_T(T)))

#define ecs_vector_add_t(vector, size, alignment) \
    _ecs_vector_add(vector, ECS_VECTOR_U(size, alignment))

FLECS_API void* _ecs_vector_addn(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_addn(vector, T, elem_count) \
    ((T*)_ecs_vector_addn(vector, ECS_VECTOR_T(T), elem_count))

#define ecs_vector_addn_t(vector, size, alignment, elem_count) \
    _ecs_vector_addn(vector, ECS_VECTOR_U(size, alignment), elem_count)

FLECS_API void* _ecs_vector_swap(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_swap(vector, T, elem_count) \
    ((T*)_ecs_vector_swap(vector, ECS_VECTOR_T(T), elem_count))

#define ecs_vector_swap_t(vector, size, alignment, elem_count) \
    _ecs_vector_swap(vector, ECS_VECTOR_U(size, alignment), elem_count)

FLECS_API void* _ecs_vector_get(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t index);

#define ecs_vector_get(vector, T, index) \
    ((T*)_ecs_vector_get(vector, ECS_VECTOR_T(T), index))

#define ecs_vector_get_t(vector, size, alignment, index) \
    _ecs_vector_get(vector, ECS_VECTOR_U(size, alignment), index)

FLECS_API void* _ecs_vector_last(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_last(vector, T) \
    (T*)_ecs_vector_last(vector, ECS_VECTOR_T(T))

FLECS_API int32_t _ecs_vector_set_min_size(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_set_min_size(vector, T, size) \
    _ecs_vector_set_min_size(vector, ECS_VECTOR_T(T), size)

FLECS_API int32_t _ecs_vector_set_min_count(
    ecs_vector_t **vector_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_set_min_count(vector, T, size) \
    _ecs_vector_set_min_count(vector, ECS_VECTOR_T(T), size)

FLECS_API void ecs_vector_remove_last(
    ecs_vector_t *vector);

FLECS_API bool _ecs_vector_pop(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    void *value);

#define ecs_vector_pop(vector, T, value) \
    _ecs_vector_pop(vector, ECS_VECTOR_T(T), value)

FLECS_API int32_t _ecs_vector_remove(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t index);

#define ecs_vector_remove(vector, T, index) \
    _ecs_vector_remove(vector, ECS_VECTOR_T(T), index)

#define ecs_vector_remove_t(vector, size, alignment, index) \
    _ecs_vector_remove(vector, ECS_VECTOR_U(size, alignment), index)

FLECS_API void _ecs_vector_reclaim(
    ecs_vector_t **vector,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_reclaim(vector, T)\
    _ecs_vector_reclaim(vector, ECS_VECTOR_T(T))

FLECS_API int32_t _ecs_vector_grow(
    ecs_vector_t **vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_grow(vector, T, size) \
    _ecs_vector_grow(vector, ECS_VECTOR_T(T), size)

FLECS_API int32_t _ecs_vector_set_size(
    ecs_vector_t **vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_set_size(vector, T, elem_count) \
    _ecs_vector_set_size(vector, ECS_VECTOR_T(T), elem_count)

#define ecs_vector_set_size_t(vector, size, alignment, elem_count) \
    _ecs_vector_set_size(vector, ECS_VECTOR_U(size, alignment), elem_count)

FLECS_API int32_t _ecs_vector_set_count(
    ecs_vector_t **vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count);

#define ecs_vector_set_count(vector, T, elem_count) \
    _ecs_vector_set_count(vector, ECS_VECTOR_T(T), elem_count)

#define ecs_vector_set_count_t(vector, size, alignment, elem_count) \
    _ecs_vector_set_count(vector, ECS_VECTOR_U(size, alignment), elem_count)

FLECS_API int32_t ecs_vector_count(
    const ecs_vector_t *vector);

FLECS_API int32_t ecs_vector_size(
    const ecs_vector_t *vector);

FLECS_API void* _ecs_vector_first(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_first(vector, T) \
    ((T*)_ecs_vector_first(vector, ECS_VECTOR_T(T)))

#define ecs_vector_first_t(vector, size, alignment) \
    _ecs_vector_first(vector, ECS_VECTOR_U(size, alignment))

FLECS_API void _ecs_vector_sort(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    ecs_comparator_t compare_action);

#define ecs_vector_sort(vector, T, compare_action) \
    _ecs_vector_sort(vector, ECS_VECTOR_T(T), compare_action)

FLECS_API void _ecs_vector_memory(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t *allocd,
    int32_t *used);

#define ecs_vector_memory(vector, T, allocd, used) \
    _ecs_vector_memory(vector, ECS_VECTOR_T(T), allocd, used)

#define ecs_vector_memory_t(vector, size, alignment, allocd, used) \
    _ecs_vector_memory(vector, ECS_VECTOR_U(size, alignment), allocd, used)

FLECS_API ecs_vector_t* _ecs_vector_copy(
    const ecs_vector_t *src,
    ecs_size_t elem_size,
    int16_t offset);

#define ecs_vector_copy(src, T) \
    _ecs_vector_copy(src, ECS_VECTOR_T(T))

#define ecs_vector_copy_t(src, size, alignment) \
    _ecs_vector_copy(src, ECS_VECTOR_U(size, alignment))

#ifndef FLECS_LEGACY
#define ecs_vector_each(vector, T, var, ...)\
    {\
        int var##_i, var##_count = ecs_vector_count(vector);\
        T* var##_array = ecs_vector_first(vector, T);\
        for (var##_i = 0; var##_i < var##_count; var##_i ++) {\
            T* var = &var##_array[var##_i];\
            __VA_ARGS__\
        }\
    }
#endif

#ifdef __cplusplus
}
#endif

#endif
