#include "private_api.h"

#define PAGE_COUNT (4096)
#define PAGE(index) ((int32_t)index >> 12)
#define OFFSET(index) ((int32_t)index & 0xFFF)
#define DATA(array, size, offset) (ECS_OFFSET(array, size * offset))

typedef struct page_t {
    void **data;
} page_t;

static
page_t* page_new(
    ecs_paged_t *paged,
    int32_t page_index)
{
    int32_t count = ecs_vector_count(paged->pages);
    page_t *pages;

    if (count <= page_index) {
        ecs_vector_set_count(&paged->pages, page_t, page_index + 1);
        pages = ecs_vector_first(paged->pages, page_t);
        ecs_os_memset(&pages[count], 0, (1 + page_index - count) * ECS_SIZEOF(page_t));
    } else {
        pages = ecs_vector_first(paged->pages, page_t);
    }

    ecs_assert(pages != NULL, ECS_INTERNAL_ERROR);

    page_t *result = &pages[page_index];
    ecs_assert(result->data == NULL, ECS_INTERNAL_ERROR);

    result->data = ecs_os_malloc(paged->column_count * ECS_SIZEOF(void*));

    /* Initialize the data arrays with zero's to guarantee that data is 
     * always initialized. When an entry is removed, data is reset back to
     * zero. Initialize now, as this can take advantage of calloc. */
    int32_t i;
    for (i = 0; i < paged->column_count; i ++) {
        result->data[i] = ecs_os_calloc(paged->column_sizes[i] * PAGE_COUNT);
        ecs_assert(result->data[i] != NULL, ECS_OUT_OF_MEMORY);
    }
    
    return result;
}

static
page_t *page_get(
    const ecs_paged_t *paged,
    int32_t page_index)
{
    ecs_assert(page_index >= 0, ECS_INTERNAL_ERROR);
    page_t *result = ecs_vector_get(paged->pages, page_t, page_index);
    if (result && !result->data) {
        return NULL;
    }

    return result;
}

static
page_t* page_ensure(
    ecs_paged_t *paged,
    int32_t page_index)
{
    page_t *page = page_get(paged, page_index);
    if (page) {
        return page;
    }

    return page_new(paged, page_index);
}

static
void page_free(
    ecs_paged_t *paged,
    page_t *page)
{
    int32_t i;
    for (i = 0; i < paged->column_count; i ++) {
        ecs_os_free(page->data[i]);
    }
}

void ecs_paged_init(
    ecs_paged_t *paged,
    int32_t column_count,
    ecs_size_t *column_sizes)
{
    ecs_size_t array_size = column_count * ECS_SIZEOF(ecs_size_t);
    paged->column_sizes = ecs_os_malloc(array_size);
    memcpy(paged->column_sizes, column_sizes, array_size);
    paged->column_count = column_count;
}

ecs_paged_t* ecs_paged_new(
    int32_t column_count,
    ecs_size_t *column_sizes)
{
    ecs_paged_t *result = ecs_os_calloc(sizeof(ecs_paged_t));
    ecs_paged_init(result, column_count, column_sizes);
    return result;
}

void ecs_paged_clear(
    ecs_paged_t *paged)
{
    ecs_assert(paged != NULL, ECS_INVALID_PARAMETER);

    ecs_vector_each(paged->pages, page_t, page, {
        page_free(paged, page);
    });

    ecs_vector_free(paged->pages);

    paged->pages = NULL;
    paged->count = 0;
}

void ecs_paged_deinit(
    ecs_paged_t *paged)
{
    ecs_assert(paged != NULL, ECS_INVALID_PARAMETER);
    ecs_os_free(paged->column_sizes);
    ecs_paged_clear(paged);
}

void ecs_paged_free(
    ecs_paged_t *paged)
{
    ecs_paged_deinit(paged);
    ecs_os_free(paged);
}

void* _ecs_paged_get(
    const ecs_paged_t *paged,
    ecs_size_t size,
    int32_t index,
    int32_t column)
{
    ecs_assert(paged != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(size == paged->column_sizes[column], ECS_INVALID_PARAMETER);
    page_t *p = page_get(paged, PAGE(index));
    if (p) {
        return ECS_OFFSET(p->data[column], size * OFFSET(index));
    }
    return NULL;
}

void* _ecs_paged_ensure(
    ecs_paged_t *paged,
    ecs_size_t size,
    int32_t index,
    int32_t column)
{
    ecs_assert(paged != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(size == paged->column_sizes[column], ECS_INVALID_PARAMETER);
    page_t *p = page_ensure(paged, PAGE(index));
    ecs_assert(p != NULL, ECS_INTERNAL_ERROR);
    return ECS_OFFSET(p->data[column], size * OFFSET(index));
}

int32_t _ecs_paged_count(
    const ecs_paged_t *paged)
{
    ecs_assert(paged != NULL, ECS_INVALID_PARAMETER);
    return paged->count;   
}

