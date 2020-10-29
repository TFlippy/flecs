#include "private_api.h"

typedef struct sparse_t {
    int32_t dense;
} sparse_t;

struct ecs_dense_t {
    ecs_vector_t *dense;        /* Dense array with indices to sparse array. The
                                 * dense array stores both alive and not alive
                                 * sparse indices. The 'count' member keeps
                                 * track of which indices are alive. */
    ecs_paged_t sparse;         /* Paged array with sparse payload */
    ecs_vector_t *data;         /* Dense payload */
    int32_t count;              /* Number of alive entries */
    int16_t size;
    int16_t alignment;
};

static
void grow_dense(
    ecs_dense_t *d)
{
    ecs_vector_add(&d->dense, uint64_t);
}

static
void assign_index(
    sparse_t *sparse,
    uint64_t * dense_array, 
    uint64_t index, 
    int32_t dense)
{
    sparse->dense = dense;
    dense_array[dense] = index;
}

static
void swap_dense(
    ecs_dense_t *d,
    sparse_t *sparse_a,
    int32_t a,
    int32_t b)
{
    ecs_assert(a != b, ECS_INTERNAL_ERROR);
    uint64_t *dense_array = ecs_vector_first(d->dense, uint64_t);
    uint64_t index_a = dense_array[a];
    uint64_t index_b = dense_array[b];

    sparse_t *sparse_b = ecs_paged_get(&d->sparse, sparse_t, ecs_to_i32(index_b), 0);
    assign_index(sparse_a, dense_array, index_a, b);
    assign_index(sparse_b, dense_array, index_b, a);
}

static
void ensure_sparse(
    ecs_dense_t *d,
    uint64_t index,
    sparse_t *sparse,
    int32_t dense)
{
    if (dense) {
        /* Check if element is alive. If element is not alive, update indices so
         * that the first unused dense element points to the sparse element. */
        int32_t count = d->count;
        if (dense == count) {
            /* If dense is the next unused element in the array, simply increase
             * the count to make it part of the alive set. */
            d->count ++;
        } else if (dense > count) {
            /* If dense is not alive, swap it with the first unused element. */
            swap_dense(d, sparse, dense, count);

            /* First unused element is now last used element */
            d->count ++;
        } else {
            /* Dense is already alive, nothing to be done */
        }
    } else {
        /* Element is not paired yet. Must add a new element to dense array */
        grow_dense(d);

        ecs_vector_t *dense_vector = d->dense;
        uint64_t *dense_array = ecs_vector_first(dense_vector, uint64_t);
        int32_t dense_count = ecs_vector_count(dense_vector) - 1;
        int32_t count = d->count ++;

        if (count < dense_count) {
            /* If there are unused elements in the list, move the first unused
             * element to the end of the list */
            uint64_t unused = dense_array[count];
            sparse_t *u_sparse = ecs_paged_get(&d->sparse, sparse_t, ecs_to_i32(unused), 0);
            assign_index(u_sparse, dense_array, unused, dense_count);
        }

        assign_index(sparse, dense_array, index, count);
    }
}

ecs_dense_t* _ecs_dense_new(
    ecs_size_t size,
    ecs_size_t alignment)
{
    ecs_dense_t *d = ecs_os_calloc(ECS_SIZEOF(ecs_dense_t));
    ecs_assert(d != NULL, ECS_OUT_OF_MEMORY);

    /* Consume first value in dense array as 0 is used in the sparse array to
     * indicate that a sparse element hasn't been paired yet. */
    ecs_vector_add(&d->dense, uint64_t);
    
    d->size = ecs_to_i16(size);
    d->count = 1;

    d->size = ecs_to_i16(size);
    d->alignment = ecs_to_i16(alignment);

    ecs_paged_init(&d->sparse, 1, &(ecs_size_t){ECS_SIZEOF(sparse_t)});

    return d;
}

int32_t ecs_dense_count(
    const ecs_dense_t *d)
{
    if (!d) {
        return 0;
    }

    return d->count - 1;
}

int32_t ecs_dense_size(
    const ecs_dense_t *d)
{
    if (!d) {
        return 0;
    }

    return ecs_vector_count(d->dense) - 1;
}

void* _ecs_dense_get(
    const ecs_dense_t *d,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(d != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || d->size == size, ECS_INVALID_PARAMETER);

    size = d->size;

    sparse_t *sparse = ecs_paged_get(&d->sparse, sparse_t, ecs_to_i32(index), 0);
    if (!sparse) {
        return NULL;
    }

    int32_t dense_index = sparse->dense;

    return ecs_vector_get_t(d->data, size, d->alignment, dense_index);
}

void* _ecs_dense_ensure(
    ecs_dense_t *d,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(d != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || d->size == size, ECS_INVALID_PARAMETER);
    ecs_assert(ecs_vector_count(d->dense) > 0, ECS_INTERNAL_ERROR);
    size = d->size;

    sparse_t *sparse = ecs_paged_ensure(&d->sparse, sparse_t, ecs_to_i32(index), 0);
    ecs_assert(sparse != NULL, ECS_INTERNAL_ERROR);
    int32_t dense = sparse->dense;

    if (dense && (dense < d->count)) {
        /* If id exists and is already alive, nothing to do */
        if (size) {
            int16_t alignment = d->alignment;
            return ecs_vector_get_t(d->data, ecs_to_i16(size), alignment, dense);
        }
    } else {
        /* Ensure id exists and is alive */
        ensure_sparse(d, index, sparse, dense);

        /* The element is guaranteed to be at the top of the dense array */
        if (size) {
            int16_t alignment = d->alignment;

            dense = d->count - 1;
            ecs_assert(index == *ecs_vector_get(d->dense, uint64_t, dense), 
                ECS_INTERNAL_ERROR);
            
            /* Ensure vector has enough elements */
            ecs_vector_set_count_t(
                &d->data, ecs_to_i16(size), alignment, dense + 1);

            return ecs_vector_get_t(d->data, ecs_to_i16(size), alignment, dense);                
        }
    }

    return NULL;
}

void ecs_dense_remove(
    ecs_dense_t *d,
    uint64_t index)
{
    ecs_assert(d != NULL, ECS_INVALID_PARAMETER);

    sparse_t *sparse = ecs_paged_get(&d->sparse, sparse_t, ecs_to_i32(index), 0);
    if (!sparse) {
        return;
    }

    int32_t dense = sparse->dense;

    if (dense) {
        int32_t count = d->count;
        if (dense == (count - 1)) {
            /* If dense is the last used element, simply decrease count */
            d->count --;
        } else if (dense < count) {
            /* If element is alive, move it to unused elements */
            swap_dense(d, sparse, dense, count - 1);
            d->count --;
        } else {
            /* Element is not alive, nothing to be done */
            return;
        }

        ecs_vector_remove_t(d->data, d->size, d->alignment, dense);
    }
}

void ecs_dense_clear(
    ecs_dense_t *d)
{
    ecs_assert(d != NULL, ECS_INVALID_PARAMETER);
    ecs_paged_deinit(&d->sparse);
    ecs_vector_set_count(&d->dense, uint64_t, 1);   
    d->count = 1;
}

void ecs_dense_free(
    ecs_dense_t *d)
{
    ecs_dense_clear(d);
    ecs_vector_free(d->dense);
    ecs_vector_free(d->data);   
    ecs_os_free(d);
}
