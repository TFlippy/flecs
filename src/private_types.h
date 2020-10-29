#ifndef FLECS_TYPES_PRIVATE_H
#define FLECS_TYPES_PRIVATE_H

#ifndef __MACH__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "flecs.h"
#include "flecs/private/entity_index.h"

#define ECS_WORLD_MAGIC (0x65637377)
#define ECS_THREAD_MAGIC (0x65637374)

typedef enum ecs_defer_kind_t {
    EcsOpNew,
    EcsOpAdd,
    EcsOpRemove,   
    EcsOpSet,
    EcsOpMut,
    EcsOpModified,
    EcsOpDelete,
    EcsOpClear
} ecs_defer_kind_t;

typedef struct ecs_defer_1_t {
    ecs_entity_t entity;
    void *value;
    ecs_size_t size;
    bool clone_value;
} ecs_defer_1_t;

typedef struct ecs_defer_n_t {
    ecs_entity_t *entities;  
    void **bulk_data;
    int32_t count;
} ecs_defer_n_t;

typedef struct ecs_defer_t {
    ecs_defer_kind_t kind;
    ecs_entity_t scope;
    ecs_entity_t component;
    ecs_entities_t components;
    union {
        ecs_defer_1_t _1;
        ecs_defer_n_t _n;
    } is;
} ecs_defer_t;

typedef enum ecs_table_eventkind_t {
    EcsTableComponentInfo
} ecs_table_eventkind_t;

typedef struct ecs_table_event_t {
    ecs_table_eventkind_t kind;
    ecs_entity_t component;
} ecs_table_event_t;

#define EcsTableHasBuiltins         1u
#define EcsTableHasComponentData    16u
#define EcsTableIsDisabled          64u
#define EcsTableHasCtors            128u
#define EcsTableHasDtors            256u
#define EcsTableHasCopy             512u
#define EcsTableHasMove             1024u
#define EcsTableHasLifecycle        (EcsTableHasCtors | EcsTableHasDtors)
#define EcsTableIsComplex           (EcsTableHasLifecycle)

typedef struct ecs_column_t {
    ecs_vector_t *data;
    int16_t size;
    int16_t alignment;
} ecs_column_t;

typedef struct ecs_data_t {
    ecs_vector_t *entities;
    ecs_vector_t *record_ptrs;
    ecs_column_t *columns;
    ecs_lifecycle_t *lc;
    int32_t *dirty_state;
    int32_t alloc_count;
} ecs_data_t;

typedef struct ecs_edge_t {
    ecs_table_t *add;
    ecs_table_t *remove;
} ecs_edge_t;

struct ecs_table_t {
    ecs_type_t type;
    ecs_ptree_t *edges;
    ecs_data_t *data;
    uint32_t id;
    ecs_flags32_t flags;
    int32_t column_count;
};

typedef struct ecs_entity_info_t {
    ecs_record_t *record;
    ecs_table_t *table;
    ecs_data_t *data;
    int32_t row;
    bool is_watched;
} ecs_entity_info_t;

typedef struct ecs_stage_t {
    ecs_world_t *world;
    int32_t id;
    int32_t defer;
    ecs_vector_t *defer_queue;
    ecs_vector_t *defer_merge_queue;
    ecs_entity_t scope;
    ecs_table_t *scope_table;
} ecs_stage_t;

typedef struct ecs_partition_t {
    ecs_sparse_t *entity_index; 
    ecs_sparse_t *tables;
    ecs_map_t *table_map;
    ecs_table_t root;
} ecs_partition_t;

typedef struct ecs_store_t {
    ecs_dense_t *sparse;
} ecs_store_t;

typedef struct ecs_thread_t {
    int32_t magic;
    ecs_world_t *world;
    ecs_stage_t *stage;
    ecs_os_thread_t thread;
} ecs_thread_t;

struct ecs_world_t {
    int32_t magic;
    ecs_lifecycle_t lc_lo[ECS_HI_COMPONENT_ID];
    ecs_map_t *lc_hi;
    ecs_ptree_t *storages;
    ecs_partition_t store;
    ecs_stage_t stage;
    ecs_stage_t temp_stage;
    ecs_world_info_t stats;
    bool in_progress;
};

#endif
