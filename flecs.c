#include "flecs.h"
#ifndef FLECS_PRIVATE_H
#define FLECS_PRIVATE_H

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

#ifndef FLECS_ENTITY_INDEX_H
#define FLECS_ENTITY_INDEX_H


#ifdef __cplusplus
extern "C" {
#endif

#define ecs_eis_get(world, entity) ecs_sparse_get((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_get_any(world, entity) ecs_sparse_get_any((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_get_or_create(world, entity) ecs_sparse_get_or_create((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_delete(world, entity) ecs_sparse_remove((world->store).entity_index, entity)
#define ecs_eis_set_generation(world, entity) ecs_sparse_set_generation((world->store).entity_index, entity)
#define ecs_eis_is_alive(world, entity) ecs_sparse_is_alive((world->store).entity_index, entity)
#define ecs_eis_exists(world, entity) ecs_sparse_exists((world->store).entity_index, entity)
#define ecs_eis_recycle(world) ecs_sparse_new_id((world->store).entity_index)
#define ecs_eis_set_size(world, size) ecs_sparse_set_size((world->store).entity_index, size)
#define ecs_eis_count(world) ecs_sparse_count((world->store).entity_index)
#define ecs_eis_clear(world) ecs_sparse_clear((world->store).entity_index)
#define ecs_eis_copy(world) ecs_sparse_copy((world->store).entity_index)
#define ecs_eis_free(world) ecs_sparse_free((world->store).entity_index)
#define ecs_eis_memory(world, allocd, used) ecs_sparse_memory((world->store).entity_index, allocd, used)

#ifdef __cplusplus
}
#endif

#endif

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


////////////////////////////////////////////////////////////////////////////////
//// Core bootstrap functions
////////////////////////////////////////////////////////////////////////////////

#define ECS_TYPE_DECL(component)\
static const ecs_entity_t __##component = ecs_typeid(component);\
ECS_VECTOR_DECL(FLECS__T##component, ecs_entity_t, 1)

#define ECS_TYPE_IMPL(component)\
ECS_VECTOR_IMPL(FLECS__T##component, ecs_entity_t, &__##component, 1)

/* Bootstrap world */
void ecs_bootstrap(
    ecs_world_t *world);

ecs_type_t ecs_bootstrap_type(
    ecs_world_t *world,
    ecs_entity_t entity);

#define ecs_bootstrap_component(world, name)\
    ecs_new_component(world, ecs_typeid(name), #name, sizeof(name), ECS_ALIGNOF(name))

#define ecs_bootstrap_tag(world, name)\
    ecs_set(world, name, EcsName, {.value = &#name[ecs_os_strlen("Ecs")], .symbol = #name});\
    ecs_add_id(world, name, ecs_role(EcsScope, ecs_get_scope(world)))


////////////////////////////////////////////////////////////////////////////////
//// Entity API
////////////////////////////////////////////////////////////////////////////////

/* Mark an entity as being watched. This is used to trigger automatic rematching
 * when entities used in system expressions change their components. */
void ecs_set_watch(
    ecs_world_t *world,
    ecs_entity_t entity);

/* Does one of the entity containers has specified component */
ecs_entity_t ecs_find_in_type(
    ecs_world_t *world,
    ecs_type_t table_type,
    ecs_entity_t component,
    ecs_entity_t flags);

/* Obtain entity info */
bool ecs_get_info(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_info_t *info);

void ecs_run_monitors(
    ecs_world_t *world, 
    ecs_table_t *dst_table,
    ecs_vector_t *v_dst_monitors, 
    int32_t dst_row, 
    int32_t count, 
    ecs_vector_t *v_src_monitors);

const char* ecs_role_str(
    ecs_entity_t entity);

size_t ecs_entity_str(
    ecs_world_t *world,
    ecs_entity_t entity,
    char *buffer,
    size_t buffer_len);


////////////////////////////////////////////////////////////////////////////////
//// World API
////////////////////////////////////////////////////////////////////////////////

/* Notify systems that there is a new table, which triggers matching */
void ecs_notify_queries_of_table(
    ecs_world_t *world,
    ecs_table_t *table);

/* Get current thread-specific stage */
ecs_stage_t* ecs_get_stage(
    ecs_world_t **world_ptr);

/* Get component callbacks */
ecs_lifecycle_t* ecs_get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component);

/* Get or create component callbacks */
ecs_lifecycle_t* ecs_get_or_create_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component);

bool ecs_defer_op_begin(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entities_t *components,
    const void *value,
    ecs_size_t size);

void ecs_defer_flush(
    ecs_world_t *world,
    ecs_stage_t *stage);
    

////////////////////////////////////////////////////////////////////////////////
//// Stage API
////////////////////////////////////////////////////////////////////////////////

/* Initialize stage data structures */
void ecs_stage_init(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Deinitialize stage */
void ecs_stage_deinit(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Merge stage with main stage */
void ecs_stage_merge(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Begin defer for stage */
void ecs_stage_defer_begin(
    ecs_world_t *world,
    ecs_stage_t *stage);

void ecs_stage_defer_end(
    ecs_world_t *world,
    ecs_stage_t *stage);    

/* Delete table from stage */
void ecs_delete_table(
    ecs_world_t *world,
    ecs_table_t *table);


////////////////////////////////////////////////////////////////////////////////
//// Defer API
////////////////////////////////////////////////////////////////////////////////

bool ecs_defer_none(
    ecs_world_t *world,
    ecs_stage_t *stage);

bool ecs_defer_modified(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_new(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entities_t *components);

bool ecs_defer_delete(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity);

bool ecs_defer_clear(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity);

bool ecs_defer_add(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_remove(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_set(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entity_t component,
    ecs_size_t size,
    const void *value,
    void **value_out,
    bool *is_added);

void ecs_defer_flush(
    ecs_world_t *world,
    ecs_stage_t *stage);


////////////////////////////////////////////////////////////////////////////////
//// Table API
////////////////////////////////////////////////////////////////////////////////

/** Find or create table for a set of components */
ecs_table_t* ecs_table_find_or_create(
    ecs_world_t *world,
    ecs_entities_t *type);

/** Find or create table for a type */
ecs_table_t* ecs_table_from_type(
    ecs_world_t *world,
    ecs_type_t type);    

/* Get table data */
ecs_data_t *ecs_table_get_data(
    ecs_table_t *table);

/* Get or create data */
ecs_data_t *ecs_table_get_or_create_data(
    ecs_table_t *table);

/* Initialize columns for data */
ecs_data_t* ecs_init_data(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *result); 

/* Clear all entities from a table. */
void ecs_table_clear(
    ecs_world_t *world,
    ecs_table_t *table);

/* Reset a table to its initial state */
void ecs_table_reset(
    ecs_world_t *world,
    ecs_table_t *table);

/* Clear all entities from the table. Do not invoke OnRemove systems */
void ecs_table_clear_silent(
    ecs_world_t *world,
    ecs_table_t *table);

/* Clear table data. Don't call OnRemove handlers. */
void ecs_table_clear_data(
    ecs_table_t *table,
    ecs_data_t *data);    

/* Return number of entities in table. */
int32_t ecs_table_count(
    ecs_table_t *table);

/* Return number of entities in data */
int32_t ecs_table_data_count(
    ecs_data_t *data);

/* Add a new entry to the table for the specified entity */
int32_t ecs_table_append(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    ecs_entity_t entity,
    ecs_record_t *record,
    bool construct);

/* Delete an entity from the table. */
void ecs_table_delete(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t index,
    bool destruct);

/* Move a row from one table to another */
void ecs_table_move(
    ecs_world_t *world,
    ecs_entity_t dst_entity,
    ecs_entity_t src_entity,
    ecs_table_t *new_table,
    ecs_data_t *new_data,
    int32_t new_index,
    ecs_table_t *old_table,
    ecs_data_t *old_data,
    int32_t old_index);

/* Grow table with specified number of records. Populate table with entities,
 * starting from specified entity id. */
int32_t ecs_table_appendn(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t count,
    const ecs_entity_t *ids);

/* Set table to a fixed size. Useful for preallocating memory in advance. */
void ecs_table_set_size(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t count);

/* Get dirty state for table columns */
int32_t* ecs_table_get_dirty_state(
    ecs_table_t *table);

/* Initialize root table */
void ecs_init_root_table(
    ecs_world_t *world,
    ecs_table_t *root);

/* Free table */
void ecs_table_free(
    ecs_world_t *world,
    ecs_table_t *table); 

ecs_table_t *ecs_table_traverse_add(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t to_add);

ecs_table_t *ecs_table_traverse_remove(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t to_remove);

void ecs_table_mark_dirty(
    ecs_table_t *table,
    ecs_entity_t component);

ecs_entity_t ecs_component_id_from_id(
    ecs_world_t *world,
    ecs_entity_t e);

const EcsComponent* ecs_component_from_id(
    ecs_world_t *world,
    ecs_entity_t e);

int32_t ecs_table_switch_from_case(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t add);    

void ecs_table_clear_edges(
    ecs_table_t *table);

void ecs_table_notify(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_table_event_t * event);


////////////////////////////////////////////////////////////////////////////////
//// Time API
////////////////////////////////////////////////////////////////////////////////

void ecs_os_time_setup(void);

uint64_t ecs_os_time_now(void);

void ecs_os_time_sleep(
    int32_t sec, 
    int32_t nanosec);

/* Increase or reset timer resolution (Windows only) */
FLECS_API
void ecs_increase_timer_resolution(
    bool enable);


////////////////////////////////////////////////////////////////////////////////
//// Utilities
////////////////////////////////////////////////////////////////////////////////

void ecs_hash(
    const void *data,
    ecs_size_t length,
    uint64_t *result);

int32_t ecs_next_pow_of_2(
    int32_t n);

/* Convert 64 bit signed integer to 16 bit */
int8_t ecs_to_i8(
    int64_t v);

/* Convert 64 bit signed integer to 16 bit */
int16_t ecs_to_i16(
    int64_t v);

/* Convert 64 bit unsigned integer to 32 bit */
uint32_t ecs_to_u32(
    uint64_t v);        

/* Convert 64 bit unsigned integer to signed 32 bit */
int32_t ecs_to_i32(
    uint64_t size);

/* Convert signed integer to size_t */
size_t ecs_to_size_t(
    int64_t size);


/* Convert size_t to ecs_size_t */
ecs_size_t ecs_from_size_t(
    size_t size);    

/* Convert 64bit value to ecs_record_t type. ecs_record_t is stored as 64bit int in the
 * entity index */
ecs_record_t ecs_to_row(
    uint64_t value);

/* Get 64bit integer from ecs_record_t */
uint64_t ecs_from_row(
    ecs_record_t record);

/* Get actual row from record row */
int32_t ecs_record_to_row(
    int32_t row, 
    bool *is_watched_out);

/* Convert actual row to record row */
int32_t ecs_row_to_record(
    int32_t row, 
    bool is_watched);

/* Convert type to entity array */
ecs_entities_t ecs_type_to_entities(
    ecs_type_t type); 

/* Convert a symbol name to an entity name by removing the prefix */
const char* ecs_name_from_symbol(
    ecs_world_t *world,
    const char *type_name); 

/* Lookup an entity by name with a specific id */
ecs_entity_t ecs_lookup_w_id(
    ecs_world_t *world,
    ecs_entity_t e,
    const char *name);

/* Set entity name with symbol */
void ecs_set_symbol(
    ecs_world_t *world,
    ecs_entity_t e,
    const char *name);

#define assert_func(cond) _assert_func(cond, #cond, __FILE__, __LINE__, __func__)
void _assert_func(
    bool cond,
    const char *cond_str,
    const char *file,
    int32_t line,
    const char *func);

#endif

static
char *ecs_vasprintf(
    const char *fmt,
    va_list args)
{
    ecs_size_t size = 0;
    char *result  = NULL;
    va_list tmpa;

    va_copy(tmpa, args);

    size = vsnprintf(result, ecs_to_size_t(size), fmt, tmpa);

    va_end(tmpa);

    if ((int32_t)size < 0) { 
        return NULL; 
    }

    result = (char *) ecs_os_malloc(size + 1);

    if (!result) { 
        return NULL; 
    }

    ecs_os_vsprintf(result, fmt, args);

    return result;
}

static int trace_indent = 0;
static int trace_level = 0;

void ecs_log_print(
    int level,
    const char *file,
    int32_t line,
    const char *fmt,
    va_list valist)
{
    (void)level;
    (void)line;

    if (level > trace_level) {
        return;
    }

    /* Massage filename so it doesn't take up too much space */
    char filebuff[256];
    ecs_os_strcpy(filebuff, file);
    file = filebuff;
    char *file_ptr = strrchr(file, '/');
    if (file_ptr) {
        file = file_ptr + 1;
    }

    /* Extension is likely the same for all files */
    file_ptr = strrchr(file, '.');
    if (file_ptr) {
        *file_ptr = '\0';
    }

    char indent[32];
    int i;
    for (i = 0; i < trace_indent; i ++) {
        indent[i * 2] = '|';
        indent[i * 2 + 1] = ' ';
    }
    indent[i * 2] = '\0';

    char *msg = ecs_vasprintf(fmt, valist);

    if (level >= 0) {
        ecs_os_log("%sinfo%s: %s%s%s%s",
            ECS_MAGENTA, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    } else if (level == -2) {
        ecs_os_warn("%swarn%s: %s%s%s%s",
            ECS_YELLOW, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    } else if (level <= -2) {
        ecs_os_err("%serr %s: %s%s%s%s",
            ECS_RED, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    }

    ecs_os_free(msg);
}

void _ecs_trace(
    int level,
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(level, file, line, fmt, valist);
}

void _ecs_warn(
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(-2, file, line, fmt, valist);
}

void _ecs_err(
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(-3, file, line, fmt, valist);
}

void ecs_log_push(void) {
    trace_indent ++;
}

void ecs_log_pop(void) {
    trace_indent --;
}

void ecs_tracing_enable(
    int level)
{
    trace_level = level;
}

static
const char* strip_file(
    const char *file)
{
    const char *f = strrchr(file, '/');
    if (f) {
        return f + 1;
    } else {
        return file;
    }
}

void _ecs_abort(
    int32_t error_code,
    const char *file,
    int32_t line)
{
    ecs_err("abort %s:%d: %s", strip_file(file), line, ecs_strerror(error_code));
    ecs_os_abort();
}

void _ecs_assert(
    bool condition,
    int32_t error_code,
    const char *condition_str,
    const char *file,
    int32_t line)
{
    if (!condition) {
        ecs_err("assert(%s) %s:%d: %s", condition_str, strip_file(file), line, 
            ecs_strerror(error_code));
        ecs_os_abort();
    }
}

const char* ecs_strerror(
    int32_t error_code)
{
    switch (error_code) {

    case ECS_INTERNAL_ERROR:
        return "ECS_INTERNAL_ERROR";
    case ECS_INVALID_OPERATION:
        return "ECS_INVALID_OPERATION";
    case ECS_INVALID_PARAMETER:
        return "ECS_INVALID_PARAMETER";
    case ECS_INVALID_ID:
        return "ECS_INVALID_ID";
    case ECS_INVALID_COMPONENT:
        return "ECS_INVALID_COMPONENT";
    case ECS_OUT_OF_MEMORY:
        return "ECS_OUT_OF_MEMORY";
    case ECS_MISSING_OS_API:
        return "ECS_MISSING_OS_API";
    case ECS_INCONSISTENT_COMPONENT_ACTION:
        return "ECS_INCONSISTENT_COMPONENT_ACTION";
    case ECS_INVALID_FROM_WORKER:
        return "ECS_INVALID_FROM_WORKER";
    }

    return "unknown error code";
}

static
ecs_flags32_t get_component_action_flags(
    ecs_lifecycle_t *lc) 
{
    ecs_flags32_t flags = 0;

    if (lc->ctor) {
        flags |= EcsTableHasCtors;
    }
    if (lc->dtor) {
        flags |= EcsTableHasDtors;
    }
    if (lc->copy) {
        flags |= EcsTableHasCopy;
    }
    if (lc->move) {
        flags |= EcsTableHasMove;
    } 

    return flags;  
}

/* Check if table has instance of component, including traits */
static
bool has_component(
    ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t component)
{
    ecs_entity_t *entities = ecs_vector_first(type, ecs_entity_t);
    int32_t i, count = ecs_vector_count(type);

    for (i = 0; i < count; i ++) {
        if (component == ecs_component_id_from_id(world, entities[i])) {
            return true;
        }
    }
    
    return false;
}

static
void notify_component_info(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t component)
{
    ecs_type_t table_type = table->type;
    if (!component || has_component(world, table_type, component)){
        int32_t column_count = ecs_vector_count(table_type);
        ecs_assert(!component || column_count != 0, ECS_INTERNAL_ERROR);

        if (!column_count) {
            return;
        }

        ecs_data_t *data = ecs_table_get_data(table);
        if (!data) {
            return;
        }
        
        if (!data->lc) {
            data->lc = ecs_os_calloc(
                ECS_SIZEOF(ecs_lifecycle_t) * column_count);
        }

        /* Reset lifecycle flags before recomputing */
        table->flags &= ~EcsTableHasLifecycle;

        /* Recompute lifecycle flags */
        ecs_entity_t *array = ecs_vector_first(table_type, ecs_entity_t);
        int32_t i;
        for (i = 0; i < column_count; i ++) {
            ecs_entity_t c = ecs_component_id_from_id(world, array[i]);
            if (!c) {
                continue;
            }
            
            ecs_lifecycle_t *lc = ecs_get_lifecycle(world, c);
            if (lc) {
                ecs_flags32_t flags = get_component_action_flags(lc);
                table->flags |= flags;
                data->lc[i] = *lc;
            }            
        }        
    }
}


ecs_data_t* ecs_init_data(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *result)
{
    ecs_type_t type = table->type; 
    int32_t i, count = table->column_count;

    /* Root tables don't have columns */
    if (!count) {
        result->columns = NULL;
        return result;
    }

    /* When a table is going to store data, initialize the cache with component
     * lifecycle callbacks */
    notify_component_info(world, table, 0);

    ecs_entity_t *entities = ecs_vector_first(type, ecs_entity_t);

    if (count) {
        result->columns = ecs_os_calloc(ECS_SIZEOF(ecs_column_t) * count);    
    } else if (count) {
        /* If a table has switch columns, store vector with the case values
            * as a regular column, so it's easier to access for systems. To
            * enable this, we need to allocate more space. */
        int32_t type_count = ecs_vector_count(type);
        result->columns = ecs_os_calloc(ECS_SIZEOF(ecs_column_t) * type_count);
    }

    if (count) {
        for (i = 0; i < count; i ++) {
            ecs_entity_t e = entities[i];

            /* Is the column a component? */
            const EcsComponent *component = ecs_component_from_id(world, e);
            if (component) {
                /* Is the component associated wit a (non-empty) type? */
                if (component->size) {
                    /* This is a regular component column */
                    result->columns[i].size = ecs_to_i16(component->size);
                    result->columns[i].alignment = ecs_to_i16(component->alignment);
                } else {
                    /* This is a tag */
                }
            } else {
                /* This is an entity that was added to the type */
            }
        }
    }

    return result;
}

/* -- Private functions -- */

static
ecs_data_t* get_data_intern(
    ecs_table_t *table,
    bool create)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);

    ecs_data_t *data = table->data;
    if (data) {
        return data;
    }

    if (!data && !create) {
        return NULL;
    }

    return table->data = ecs_os_calloc(ECS_SIZEOF(ecs_data_t));
}

ecs_data_t* ecs_table_get_data(
    ecs_table_t *table)
{
    return get_data_intern(table, false);
}

ecs_data_t* ecs_table_get_or_create_data(
    ecs_table_t *table)
{
    return get_data_intern(table, true);
}

static
void ctor_component(
    ecs_lifecycle_t * lc,
    ecs_column_t * column,
    int32_t row,
    int32_t count)
{
    ecs_xtor_t ctor;
    if (lc && (ctor = lc->ctor)) {
        int16_t size = column->size;
        int16_t alignment = column->alignment;
        void *ptr = ecs_vector_get_t(column->data, size, alignment, row);
        ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);
        ctor(ptr, ecs_to_size_t(size), count);
    }
}

static
void dtor_component(
    ecs_lifecycle_t * lc,
    ecs_column_t * column,
    int32_t row,
    int32_t count)
{
    ecs_xtor_t dtor;
    if (lc && (dtor = lc->dtor)) {
        int16_t size = column->size;
        int16_t alignment = column->alignment;    
        void *ptr = ecs_vector_get_t(column->data, size, alignment, row);
        ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);
        dtor(ptr, ecs_to_size_t(size), count);
    }
}

static
void dtor_all_components(
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t row,
    int32_t count)
{
    int32_t column_count = table->column_count;
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &data->columns[i];
        dtor_component(&data->lc[i], column, row, count);
    }
}

static
void run_remove_actions(
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t row,
    int32_t count)
{
    if (count) {
        dtor_all_components(table, data, row, count);
    }
}

void ecs_table_clear_data(
    ecs_table_t * table,
    ecs_data_t * data)
{
    if (!data) {
        return;
    }
    
    ecs_column_t *columns = data->columns;
    if (columns) {
        int32_t c, column_count = table->column_count;
        for (c = 0; c < column_count; c ++) {
            ecs_vector_free(columns[c].data);
        }
        ecs_os_free(columns);
        data->columns = NULL;
    }

    ecs_vector_free(data->entities);
    ecs_vector_free(data->record_ptrs);
    ecs_os_free(data->dirty_state);

    data->entities = NULL;
    data->record_ptrs = NULL;
}

/* Clear columns. Deactivate table in systems if necessary, but do not invoke
 * OnRemove handlers. This is typically used when restoring a table to a
 * previous state. */
void ecs_table_clear_silent(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_data_t *data = ecs_table_get_data(table);
    if (!data) {
        return;
    }

    int32_t count = ecs_vector_count(data->entities);
    
    ecs_table_clear_data(table, table->data);

    if (count) {
        // ecs_table_activate(world, table, 0, false);
    }
}

/* Delete all entities in table, invoke OnRemove handlers. This function is used
 * when an application invokes delete_w_filter. Use ecs_table_clear_silent, as 
 * the table may have to be deactivated with systems. */
void ecs_table_clear(
    ecs_world_t * world,
    ecs_table_t * table)
{
    ecs_data_t *data = ecs_table_get_data(table);
    if (data) {
        run_remove_actions(table, data, 0, ecs_table_data_count(data));

        ecs_entity_t *entities = ecs_vector_first(data->entities, ecs_entity_t);
        int32_t i, count = ecs_vector_count(data->entities);
        for(i = 0; i < count; i ++) {
            ecs_eis_delete(world, entities[i]);
        }
    }

    ecs_table_clear_silent(world, table);
}

/* Free table resources. Do not invoke handlers and do not activate/deactivate
 * table with systems. This function is used when the world is freed. */
void ecs_table_free(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;
    ecs_data_t *data = ecs_table_get_data(table);
    if (data) {
        run_remove_actions(table, data, 0, ecs_table_data_count(data));
        if (data->lc) {
            ecs_os_free(data->lc);
        }            
    }

    ecs_table_clear_data(table, table->data);
    ecs_table_clear_edges(table);
    
    ecs_ptiny_free(table->edges);
    ecs_vector_free((ecs_vector_t*)table->type);

    table->id = 0;

    ecs_os_free(table->data);
}

/* Reset a table to its initial state. */
void ecs_table_reset(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_ptiny_free(table->edges);
    table->edges = ecs_ptiny_new(ecs_edge_t);
}

static
void mark_dirty(
    ecs_data_t *data,
    int32_t index)
{
    if (data->dirty_state) {
        data->dirty_state[index] ++;
    }
}

void ecs_table_mark_dirty(
    ecs_table_t *table,
    ecs_entity_t component)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table->data != NULL, ECS_INTERNAL_ERROR);
    
    ecs_data_t *data = table->data;
    if (data->dirty_state) {
        int32_t index = ecs_type_index_of(table->type, component);
        ecs_assert(index != -1, ECS_INTERNAL_ERROR);
        mark_dirty(data, index);
    }
}

static
void ensure_data(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t * column_count_out,
    ecs_column_t ** columns_out)
{
    int32_t column_count = table->column_count;
    ecs_column_t *columns = NULL;

    /* It is possible that the table data was created without content. 
     * Now that data is going to be written to the table, initialize */ 
    if (column_count) {
        columns = data->columns;

        if (!columns) {
            ecs_init_data(world, table, data);
            columns = data->columns;
        }

        *column_count_out = column_count;
        *columns_out = columns;
    }
}

static
void grow_column(
    ecs_column_t * column,
    ecs_lifecycle_t * lc,
    int32_t to_add,
    int32_t new_size,
    bool construct)
{
    ecs_vector_t *vec = column->data;
    int16_t alignment = column->alignment;

    int32_t size = column->size;
    int32_t count = ecs_vector_count(vec);
    int32_t old_size = ecs_vector_size(vec);
    int32_t new_count = count + to_add;
    bool can_realloc = new_size != old_size;

    ecs_assert(new_size >= new_count, ECS_INTERNAL_ERROR);

    /* If the array could possibly realloc and the component has a move action 
     * defined, move old elements manually */
    ecs_move_t move;
    if (lc && count && can_realloc && (move = lc->move)) {
        ecs_xtor_t ctor = lc->ctor;
        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);

        /* Create new vector */
        ecs_vector_t *new_vec = ecs_vector_new_t(size, alignment, new_size);
        ecs_vector_set_count_t(&new_vec, size, alignment, new_count);

        void *old_buffer = ecs_vector_first_t(
            vec, size, alignment);

        void *new_buffer = ecs_vector_first_t(
            new_vec, size, alignment);

        /* First construct elements (old and new) in new buffer */
        ctor(new_buffer, ecs_to_size_t(size), construct ? new_count : count);
        
        /* Move old elements */
        move(new_buffer, old_buffer, ecs_to_size_t(size), count);

        /* Free old vector */
        ecs_vector_free(vec);
        column->data = new_vec;
    } else {
        /* If array won't realloc or has no move, simply add new elements */
        if (can_realloc) {
            ecs_vector_set_size_t(&vec, size, alignment, new_size);
        }

        void *elem = ecs_vector_addn_t(&vec, size, alignment, to_add);

        ecs_xtor_t ctor;
        if (construct && lc && (ctor = lc->ctor)) {
            /* If new elements need to be constructed and component has a
             * constructor, construct */
            ctor(elem, ecs_to_size_t(size), to_add);
        }

        column->data = vec;
    }

    ecs_assert(ecs_vector_size(column->data) == new_size, 
        ECS_INTERNAL_ERROR);
}

static
int32_t grow_data(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t to_add,
    int32_t size,
    const ecs_entity_t *ids)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    int32_t cur_count = ecs_table_data_count(data);
    int32_t column_count = table->column_count;
    ecs_column_t *columns;
    ensure_data(world, table, data, &column_count, &columns);

    /* Add record to record ptr array */
    ecs_vector_set_size(&data->record_ptrs, ecs_record_t*, size);
    ecs_record_t **r = ecs_vector_addn(&data->record_ptrs, ecs_record_t*, to_add);
    ecs_assert(r != NULL, ECS_INTERNAL_ERROR);
    if (ecs_vector_size(data->record_ptrs) > size) {
        size = ecs_vector_size(data->record_ptrs);
    }

    /* Add entity to column with entity ids */
    ecs_vector_set_size(&data->entities, ecs_entity_t, size);
    ecs_entity_t *e = ecs_vector_addn(&data->entities, ecs_entity_t, to_add);
    ecs_assert(e != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_vector_size(data->entities) == size, ECS_INTERNAL_ERROR);

    /* Initialize entity ids and record ptrs */
    int32_t i;
    if (ids) {
        for (i = 0; i < to_add; i ++) {
            e[i] = ids[i];
        }
    } else {
        ecs_os_memset(e, 0, ECS_SIZEOF(ecs_entity_t) * to_add);
    }
    ecs_os_memset(r, 0, ECS_SIZEOF(ecs_record_t*) * to_add);

    /* Add elements to each column array */
    ecs_lifecycle_t *lc_array = data->lc;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        if (!column->size) {
            continue;
        }

        ecs_lifecycle_t *lc = NULL;
        if (lc_array) {
            lc = &lc_array[i];
        }

        grow_column(column, lc, to_add, size, true);
        ecs_assert(ecs_vector_size(columns[i].data) == size, 
            ECS_INTERNAL_ERROR);
    }

    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    if (!world->in_progress && !cur_count) {
        // ecs_table_activate(world, table, 0, true);
    }

    data->alloc_count ++;

    /* Return index of first added entity */
    return cur_count;
}

static
void fast_append(
    ecs_column_t *columns,
    int32_t column_count)
{
    /* Add elements to each column array */
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        if (size) {
            int16_t alignment = column->alignment;
            ecs_vector_add_t(&column->data, size, alignment);
        }
    }
}

int32_t ecs_table_append(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    ecs_entity_t entity,
    ecs_record_t * record,
    bool construct)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    /* Get count & size before growing entities array. This tells us whether the
     * arrays will realloc */
    int32_t count = ecs_vector_count(data->entities);
    int32_t size = ecs_vector_size(data->entities);

    int32_t column_count = table->column_count;
    ecs_column_t *columns;
    ensure_data(world, table, data, &column_count, &columns);

    /* Grow buffer with entity ids, set new element to new entity */
    ecs_entity_t *e = ecs_vector_add(&data->entities, ecs_entity_t);
    ecs_assert(e != NULL, ECS_INTERNAL_ERROR);
    *e = entity;    

    /* Keep track of alloc count. This allows references to check if cached
     * pointers need to be updated. */  
    data->alloc_count += (count == size);

    /* Add record ptr to array with record ptrs */
    ecs_record_t **r = ecs_vector_add(&data->record_ptrs, ecs_record_t*);
    ecs_assert(r != NULL, ECS_INTERNAL_ERROR);
    *r = record;
 
    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    /* If this is the first entity in this table, signal queries so that the
     * table moves from an inactive table to an active table. */
    if (!world->in_progress && !count) {
        // ecs_table_activate(world, table, 0, true);
    }

    ecs_assert(count >= 0, ECS_INTERNAL_ERROR);

    /* Fast path: no switch columns, no lifecycle actions */
    if (!(table->flags & EcsTableIsComplex)) {
        fast_append(columns, column_count);
        return count;
    }

    ecs_lifecycle_t *lc_array = data->lc;

    /* Reobtain size to ensure that the columns have the same size as the 
     * entities and record vectors. This keeps reasoning about when allocations
     * occur easier. */
    size = ecs_vector_size(data->entities);

    /* Grow component arrays with 1 element */
    int32_t i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        if (!column->size) {
            continue;
        }

        ecs_lifecycle_t *lc = NULL;
        if (lc_array) {
            lc = &lc_array[i];
        }

        grow_column(column, lc, 1, size, construct);
        
        ecs_assert(
            ecs_vector_size(columns[i].data) == ecs_vector_size(data->entities), 
            ECS_INTERNAL_ERROR); 
            
        ecs_assert(
            ecs_vector_count(columns[i].data) == ecs_vector_count(data->entities), 
            ECS_INTERNAL_ERROR);                        
    }

    return count;
}

static
void fast_delete_last(
    ecs_column_t *columns,
    int32_t column_count) 
{
    int i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        ecs_vector_remove_last(column->data);
    }
}

static
void fast_delete(
    ecs_column_t *columns,
    int32_t column_count,
    int32_t index) 
{
    int i;
    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        if (size) {
            int16_t alignment = column->alignment;
            ecs_vector_remove_t(column->data, size, alignment, index);
        } 
    }
}

void ecs_table_delete(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t index,
    bool destruct)
{
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_vector_t *entity_column = data->entities;
    int32_t count = ecs_vector_count(entity_column);

    ecs_assert(count > 0, ECS_INTERNAL_ERROR);
    count --;
    
    ecs_assert(index <= count, ECS_INTERNAL_ERROR);

    ecs_lifecycle_t *lc_array = data->lc;
    int32_t column_count = table->column_count;
    int32_t i;

    ecs_entity_t *entities = ecs_vector_first(entity_column, ecs_entity_t);
    ecs_entity_t entity_to_move = entities[count];

    /* Move last entity id to index */
    entities[index] = entity_to_move;
    ecs_vector_remove_last(entity_column);

    /* Move last record ptr to index */
    ecs_vector_t *record_column = data->record_ptrs;     
    ecs_record_t **records = ecs_vector_first(record_column, ecs_record_t*);

    ecs_assert(count < ecs_vector_count(record_column), ECS_INTERNAL_ERROR);
    ecs_record_t *record_to_move = records[count];

    records[index] = record_to_move;
    ecs_vector_remove_last(record_column);    

    /* Update record of moved entity in entity index */
    if (index != count) {
        if (record_to_move) {
            if (record_to_move->row >= 0) {
                record_to_move->row = index + 1;
            } else {
                record_to_move->row = -(index + 1);
            }
            ecs_assert(record_to_move->table != NULL, ECS_INTERNAL_ERROR);
            ecs_assert(record_to_move->table == table, ECS_INTERNAL_ERROR);
        }
    } 

    /* If the table is monitored indicate that there has been a change */
    mark_dirty(data, 0);

    if (!count) {
        // ecs_table_activate(world, table, NULL, false);
    }

    /* Move each component value in array to index */
    ecs_column_t *columns = data->columns;

    if (!(table->flags & EcsTableIsComplex)) {
        if (index == count) {
            fast_delete_last(columns, column_count);
        } else {
            fast_delete(columns, column_count, index);
        }
        return;
    }

    for (i = 0; i < column_count; i ++) {
        ecs_column_t *column = &columns[i];
        int16_t size = column->size;
        int16_t alignment = column->alignment;
        if (size) {
            ecs_lifecycle_t *lc = lc_array ? &lc_array[i] : NULL;
            ecs_xtor_t dtor;

            void *dst = ecs_vector_get_t(column->data, size, alignment, index);

            ecs_move_t move;
            if (lc && (count != index) && (move = lc->move)) {
                void *src = ecs_vector_get_t(column->data, size, alignment, count);

                /* If the delete is not destructing the component, the component
                * was already deleted, most likely by a move. In that case we
                * still need to move, but we need to make sure we're moving
                * into an element that is initialized with valid memory, so
                * call the constructor. */
                if (!destruct) {
                    ecs_xtor_t ctor = lc->ctor;
                    ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);
                    ctor(dst, ecs_to_size_t(size), 1);   
                }

                /* Move last element into deleted element */
                move(dst, src, ecs_to_size_t(size), 1);

                /* Memory has been copied, we can now simply remove last */
                ecs_vector_remove_last(column->data);                              
            } else {
                if (destruct && lc && (dtor = lc->dtor)) {
                    dtor(dst, ecs_to_size_t(size), 1);
                }

                ecs_vector_remove_t(column->data, size, alignment, index);
            }
        }
    }
}

static
void fast_move(
    ecs_table_t * new_table,
    ecs_data_t * new_data,
    int32_t new_index,
    ecs_table_t * old_table,
    ecs_data_t * old_data,
    int32_t old_index)
{
    ecs_type_t new_type = new_table->type;
    ecs_type_t old_type = old_table->type;

    int32_t i_new = 0, new_column_count = new_table->column_count;
    int32_t i_old = 0, old_column_count = old_table->column_count;
    ecs_entity_t *new_components = ecs_vector_first(new_type, ecs_entity_t);
    ecs_entity_t *old_components = ecs_vector_first(old_type, ecs_entity_t);

    ecs_column_t *old_columns = old_data->columns;
    ecs_column_t *new_columns = new_data->columns;

    for (; (i_new < new_column_count) && (i_old < old_column_count);) {
        ecs_entity_t new_component = new_components[i_new];
        ecs_entity_t old_component = old_components[i_old];

        if (new_component == old_component) {
            ecs_column_t *new_column = &new_columns[i_new];
            ecs_column_t *old_column = &old_columns[i_old];
            int16_t size = new_column->size;

            if (size) {
                int16_t alignment = new_column->alignment;
                void *dst = ecs_vector_get_t(new_column->data, size, alignment, new_index);
                void *src = ecs_vector_get_t(old_column->data, size, alignment, old_index);

                ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);
                ecs_assert(src != NULL, ECS_INTERNAL_ERROR);
                ecs_os_memcpy(dst, src, size); 
            }
        }

        i_new += new_component <= old_component;
        i_old += new_component >= old_component;
    }
}

void ecs_table_move(
    ecs_world_t * world,
    ecs_entity_t dst_entity,
    ecs_entity_t src_entity,
    ecs_table_t *new_table,
    ecs_data_t *new_data,
    int32_t new_index,
    ecs_table_t *old_table,
    ecs_data_t *old_data,
    int32_t old_index)
{
    (void)world;
    
    ecs_assert(new_table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(old_table != NULL, ECS_INTERNAL_ERROR);

    ecs_assert(old_index >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(new_index >= 0, ECS_INTERNAL_ERROR);

    ecs_assert(old_data != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(new_data != NULL, ECS_INTERNAL_ERROR);

    if (!((new_table->flags | old_table->flags) & EcsTableIsComplex)) {
        fast_move(new_table, new_data, new_index, old_table, old_data, old_index);
        return;
    }

    bool same_entity = dst_entity == src_entity;

    ecs_type_t new_type = new_table->type;
    ecs_type_t old_type = old_table->type;

    int32_t i_new = 0, new_column_count = new_table->column_count;
    int32_t i_old = 0, old_column_count = old_table->column_count;
    ecs_entity_t *new_components = ecs_vector_first(new_type, ecs_entity_t);
    ecs_entity_t *old_components = ecs_vector_first(old_type, ecs_entity_t);

    ecs_column_t *old_columns = old_data->columns;
    ecs_column_t *new_columns = new_data->columns;

    for (; (i_new < new_column_count) && (i_old < old_column_count);) {
        ecs_entity_t new_component = new_components[i_new];
        ecs_entity_t old_component = old_components[i_old];

        if (new_component == old_component) {
            ecs_column_t *new_column = &new_columns[i_new];
            ecs_column_t *old_column = &old_columns[i_old];
            int16_t size = new_column->size;
            int16_t alignment = new_column->alignment;

            if (size) {
                void *dst = ecs_vector_get_t(new_column->data, size, alignment, new_index);
                void *src = ecs_vector_get_t(old_column->data, size, alignment, old_index);

                ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);
                ecs_assert(src != NULL, ECS_INTERNAL_ERROR);

                ecs_lifecycle_t *lc = &new_data->lc[i_new];
                if (same_entity) {
                    ecs_move_t move;
                    if (lc && (move = lc->move)) {
                        ecs_xtor_t ctor = lc->ctor;

                        /* Ctor should always be set if copy is set */
                        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);

                        /* Construct a new value, move the value to it */
                        ctor(dst, ecs_to_size_t(size), 1);
                        move(dst, src, ecs_to_size_t(size), 1);
                    } else {
                        ecs_os_memcpy(dst, src, size);
                    }
                } else {
                    ecs_copy_t copy;
                    if (lc && (copy = lc->copy)) {
                        ecs_xtor_t ctor = lc->ctor;

                        /* Ctor should always be set if copy is set */
                        ecs_assert(ctor != NULL, ECS_INTERNAL_ERROR);
                        ctor(dst, ecs_to_size_t(size), 1);
                        copy(dst, src, ecs_to_size_t(size), 1);
                    } else {
                        ecs_os_memcpy(dst, src, size);
                    }
                }
            }
        } else {
            if (new_component < old_component) {
                ctor_component(&new_data->lc[i_new],
                    &new_columns[i_new], new_index, 1);
            } else {
                dtor_component(&old_data->lc[i_old],
                    &old_columns[i_old], old_index, 1);
            }
        }

        i_new += new_component <= old_component;
        i_old += new_component >= old_component;
    }

    for (; (i_new < new_column_count); i_new ++) {
        ctor_component(&new_data->lc[i_new], &new_columns[i_new], new_index, 1);
    }

    for (; (i_old < old_column_count); i_old ++) {
        dtor_component(&old_data->lc[i_old], &old_columns[i_old], old_index, 1);
    }
}

int32_t ecs_table_appendn(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t to_add,
    const ecs_entity_t *ids)
{
    int32_t cur_count = ecs_table_data_count(data);
    return grow_data(world, table, data, to_add, cur_count + to_add, ids);
}

void ecs_table_set_size(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_data_t * data,
    int32_t size)
{
    int32_t cur_count = ecs_table_data_count(data);

    if (cur_count < size) {
        grow_data(world, table, data, 0, size, NULL);
    } else if (!size) {
        /* Initialize columns if 0 is passed. This is a shortcut to initialize
         * columns when, for example, an API call is inserting bulk data. */
        int32_t column_count = table->column_count;
        ecs_column_t *columns;
        ensure_data(world, table, data, &column_count, &columns);
    }
}

int32_t ecs_table_data_count(
    ecs_data_t *data)
{
    return data ? ecs_vector_count(data->entities) : 0;
}

int32_t ecs_table_count(
    ecs_table_t *table)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_data_t *data = table->data;
    if (!data) {
        return 0;
    }

    return ecs_table_data_count(data);
}

int32_t* ecs_table_get_dirty_state(
    ecs_table_t *table)
{
    ecs_data_t *data = ecs_table_get_or_create_data(table);
    if (!data->dirty_state) {
        data->dirty_state = ecs_os_calloc(ECS_SIZEOF(int32_t) * (table->column_count + 1));
        ecs_assert(data->dirty_state != NULL, ECS_INTERNAL_ERROR);
    }
    return data->dirty_state;
}

int32_t* ecs_table_get_monitor(
    ecs_table_t *table)
{
    int32_t *dirty_state = ecs_table_get_dirty_state(table);
    ecs_assert(dirty_state != NULL, ECS_INTERNAL_ERROR);

    int32_t column_count = table->column_count;
    return ecs_os_memdup(dirty_state, (column_count + 1) * ECS_SIZEOF(int32_t));
}

void ecs_table_notify(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_table_event_t * event)
{
    switch(event->kind) {
    case EcsTableComponentInfo:
        notify_component_info(world, table, event->component);
        break;
    }
}

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


static
void* get_component_w_index(
    ecs_entity_info_t *info,
    int32_t index)
{
    ecs_data_t *data = info->data;
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_column_t *columns = data->columns;
    ecs_assert(columns != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(index < info->table->column_count, ECS_INVALID_COMPONENT);

    ecs_column_t *column = &columns[index];
    ecs_vector_t *data_vec = column->data;
    int16_t size = column->size; 
    
    /* If size is 0, component does not have a value. This is likely caused by
     * an application trying to call ecs_get with a tag. */
    ecs_assert(size != 0, ECS_INVALID_PARAMETER);

    /* This function should not be called if an entity does not exist in the
     * provided table. Therefore if the component is found in the table, and an
     * entity exists for it, the vector cannot be NULL */
    ecs_assert(data_vec != NULL, ECS_INTERNAL_ERROR);

    void *ptr = ecs_vector_first_t(data_vec, size, column->alignment);

    /* This could only happen when the vector is empty, which should not be
     * possible since the vector should at least have one element */
    ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR);

    return ECS_OFFSET(ptr, info->row * size);
}

/* Get pointer to single component value */
static
void* get_component(
    ecs_entity_info_t *info,
    ecs_entity_t component)
{
    ecs_assert(info->table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(component != 0, ECS_INTERNAL_ERROR);
    ecs_assert(info->row >= 0, ECS_INTERNAL_ERROR);

    ecs_table_t *table = info->table;
    ecs_type_t type = table->type;

    ecs_entity_t *ids = ecs_vector_first(type, ecs_entity_t);

    /* The table column_count contains the maximum column index that actually
     * contains data. This excludes component ids that do not have data, such
     * as tags. Therefore it is faster to iterate column_count vs. all the
     * elements in the type.
     *
     * The downside of this is that the code can't always detect when an 
     * application attempts to get the value of a tag (which is not allowed). To
     * ensure consistent behavior in debug mode, the entire type is iterated as
     * this guarantees that the code will assert when attempting to obtain the
     * value of a tag. */
#ifndef NDEBUG
    int i, count = ecs_vector_count(type);
#else
    int i, count = table->column_count;
#endif

    for (i = 0; i < count; i ++) {
        if (ids[i] == component) {
            return get_component_w_index(info, i);
        }
    }

    return NULL;
}

/* Utility to compute actual row from row in record */
static
int32_t set_row_info(
    ecs_entity_info_t *info,
    int32_t row)
{
    return info->row = ecs_record_to_row(row, &info->is_watched);
}

/* Utility to set info from main stage record */
static
void set_info_from_record(
    ecs_entity_t e,
    ecs_entity_info_t *info,
    ecs_record_t *record)
{
    (void)e;
    
    ecs_assert(record != NULL, ECS_INTERNAL_ERROR);

    info->record = record;

    ecs_table_t *table = record->table;

    set_row_info(info, record->row);

    info->table = table;
    if (!info->table) {
        return;
    }

    ecs_data_t *data = ecs_table_get_data(table);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    info->data = data;

    ecs_assert(ecs_vector_count(data->entities) > info->row, 
        ECS_INTERNAL_ERROR);
}

/* Get info from main stage */
bool ecs_get_info(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_info_t *info)
{
    ecs_record_t *record = ecs_eis_get(world, entity);

    if (!record) {
        info->table = NULL;
        info->is_watched = false;
        info->record = NULL;
        return false;
    }

    set_info_from_record(entity, info, record);

    return true;
}

static
ecs_lifecycle_t *get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_entity_t real_id = ecs_component_id_from_id(world, component);
    if (real_id) {
        return ecs_get_lifecycle(world, real_id);
    } else {
        return NULL;
    }
}

static
int32_t new_entity(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * new_table)
{
    ecs_record_t *record = info->record;
    ecs_data_t *new_data = ecs_table_get_or_create_data(new_table);
    int32_t new_row;

    if (!record) {
        record = ecs_eis_get_or_create(world, entity);
    }

    new_row = ecs_table_append(
        world, new_table, new_data, entity, record, true);

    record->table = new_table;
    record->row = ecs_row_to_record(new_row, info->is_watched);

    ecs_assert(
        ecs_vector_count(new_data[0].entities) > new_row, 
        ECS_INTERNAL_ERROR);

    info->data = new_data;
    
    return new_row;
}

static
int32_t move_entity(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * src_table,
    ecs_data_t * src_data,
    int32_t src_row,
    ecs_table_t * dst_table)
{    
    ecs_data_t *dst_data = ecs_table_get_or_create_data(dst_table);
    ecs_assert(src_data != dst_data, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_is_alive(world, entity), ECS_INVALID_PARAMETER);
    ecs_assert(src_table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(src_data != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(src_row >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(ecs_vector_count(src_data->entities) > src_row, 
        ECS_INTERNAL_ERROR);

    ecs_record_t *record = info->record;
    ecs_assert(!record || record == ecs_eis_get(world, entity), 
        ECS_INTERNAL_ERROR);

    int32_t dst_row = ecs_table_append(world, dst_table, dst_data, entity, 
        record, false);

    record->table = dst_table;
    record->row = ecs_row_to_record(dst_row, info->is_watched);

    ecs_assert(ecs_vector_count(src_data->entities) > src_row, 
        ECS_INTERNAL_ERROR);

    /* Copy entity & components from src_table to dst_table */
    if (src_table->type) {
        ecs_table_move(world, entity, entity, dst_table, dst_data, dst_row, 
            src_table, src_data, src_row);          
    }
    
    ecs_table_delete(world, src_table, src_data, src_row, false);

    info->data = dst_data;

    return dst_row;
}

static
void delete_entity(
    ecs_world_t * world,
    ecs_table_t * src_table,
    ecs_data_t * src_data,
    int32_t src_row)
{
    ecs_table_delete(world, src_table, src_data, src_row, true);
}

static
void commit(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_table_t * dst_table)
{
    ecs_assert(!world->in_progress, ECS_INTERNAL_ERROR);
    
    ecs_table_t *src_table = info->table;
    if (src_table == dst_table) {
        return;
    }

    if (src_table) {
        ecs_data_t *src_data = info->data;
        ecs_assert(dst_table != NULL, ECS_INTERNAL_ERROR);

        if (dst_table) { 
            info->row = move_entity(world, entity, info, src_table, 
                src_data, info->row, dst_table);
            info->table = dst_table;
        } else {
            delete_entity(world, src_table, src_data, info->row);
            ecs_record_t *r = ecs_eis_get_or_create(world, entity);
            r->table = NULL;
            r->row = (info->is_watched == true) * -1;
        }      
    } else {        
        if (dst_table) {
            info->row = new_entity(world, entity, info, dst_table);
            info->table = dst_table;
        }        
    } 
}

static
void add_w_info(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_info_t * info,
    ecs_entity_t component)
{
    ecs_table_t *dst_table = ecs_table_traverse_add(
        world, info->table, component);

    commit(world, entity, info, dst_table);
}

static
void *get_mutable(
    ecs_world_t * world,
    ecs_entity_t entity,
    ecs_entity_t component,
    ecs_entity_info_t * info,
    bool * is_added)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER);

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        return _ecs_dense_ensure(sparse, 0, entity);
    }

    void *dst = NULL;
    if (ecs_get_info(world, entity, info) && info->table) {
        dst = get_component(info, component);
    }

    if (!dst) {
        ecs_table_t *table = info->table;

        add_w_info(world, entity, info, component);
        ecs_get_info(world, entity, info);
        ecs_assert(info->table != NULL, ECS_INTERNAL_ERROR);

        dst = get_component(info, component);
        if (is_added) {
            *is_added = table != info->table;
        }
        
        return dst;
    } else {
        if (is_added) {
            *is_added = false;
        }

        return dst;
    }
}

int32_t ecs_record_to_row(
    int32_t row, 
    bool *is_watched_out) 
{
    bool is_watched = row < 0;
    row = row * -(is_watched * 2 - 1) - 1 * (row != 0);
    *is_watched_out = is_watched;
    return row;
}

int32_t ecs_row_to_record(
    int32_t row, 
    bool is_watched) 
{
    return (row + 1) * -(is_watched * 2 - 1);
}

ecs_entity_t ecs_new_id(
    ecs_world_t *world)
{
    ecs_entity_t entity;

    entity = ecs_eis_recycle(world);

    return entity;
}

ecs_entity_t ecs_new_component_id(
    ecs_world_t *world)
{    
    if (world->stats.last_component_id >= ECS_HI_COMPONENT_ID) {
        /* If the low component ids are depleted, return a regular entity id */
        return ecs_new_id(world);
    } else {
        return world->stats.last_component_id ++;
    }
}

void ecs_add_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER);
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_add(world, stage, entity, component)) {
        return;
    }

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        _ecs_dense_ensure(sparse, 0, entity);
        ecs_defer_flush(world, stage);
        return;
    }

    ecs_entity_info_t info;
    ecs_get_info(world, entity, &info);

    ecs_table_t *src_table = info.table;
    ecs_table_t *dst_table = ecs_table_traverse_add(
        world, src_table, component);

    commit(world, entity, &info, dst_table);

    ecs_defer_flush(world, stage);
}

void ecs_remove_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(component != 0, ECS_INVALID_PARAMETER); 
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_remove(world, stage, entity, component)) {
        return;
    }

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        ecs_dense_remove(sparse, entity);
        ecs_defer_flush(world, stage);
        return;
    }

    ecs_entity_info_t info;
    ecs_get_info(world, entity, &info);

    ecs_table_t *src_table = info.table;
    ecs_table_t *dst_table = ecs_table_traverse_remove(
        world, src_table, component);

    commit(world, entity, &info, dst_table);

    ecs_defer_flush(world, stage);
}

void ecs_clear(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(entity != 0, ECS_INVALID_PARAMETER);

    ecs_stage_t *stage = ecs_get_stage(&world);
    if (ecs_defer_clear(world, stage, entity)) {
        return;
    }

    ecs_entity_info_t info;
    info.table = NULL;
    ecs_get_info(world, entity, &info);
    commit(world, entity, &info, NULL);

    ecs_defer_flush(world, stage);
}

void ecs_delete(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(entity != 0, ECS_INVALID_PARAMETER);

    ecs_stage_t *stage = ecs_get_stage(&world);
    if (ecs_defer_delete(world, stage, entity)) {
        return;
    }

    ecs_record_t *r = ecs_sparse_remove_get(
        world->store.entity_index, ecs_record_t, entity);

    if (r) {
        ecs_entity_info_t info;
        set_info_from_record(entity, &info, r);

        ecs_table_t *table = info.table;
        if (table) {
            delete_entity(world, table, info.data, info.row);
            r->table = NULL;
        }

        r->row = 0;
    }

    ecs_ptree_iter_t it = ecs_ptree_iter(world->storages);
    ecs_store_t *store;
    while ((store = ecs_ptree_next(&it, ecs_store_t))) {
        ecs_dense_t *dense;
        if ((dense = store->sparse)) {
            ecs_dense_remove(dense, entity);
        }
    }

    ecs_defer_flush(world, stage);
}

const void* ecs_get_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(world->magic == ECS_WORLD_MAGIC, ECS_INTERNAL_ERROR);

    ecs_store_t *store = ecs_ptree_get(world->storages, ecs_store_t, component);
    ecs_dense_t *sparse;
    if (store && (sparse = store->sparse)) {
        return _ecs_dense_get(sparse, 0, entity);
    }

    ecs_entity_info_t info;
    void *ptr = NULL;
    bool found = ecs_get_info(world, entity, &info);
    if (found) {
        if (!info.table) {
            return NULL;
        }

        ptr = get_component(&info, component);        
    }

    return ptr;
}

void* ecs_get_mut_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    bool * is_added)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    void *result;

    if (ecs_defer_set(
        world, stage, EcsOpMut, entity, component, 0, NULL, &result, is_added))
    {
        return result;
    }

    ecs_entity_info_t info;
    result = get_mutable(world, entity, component, &info, is_added);
    
    ecs_defer_flush(world, stage);

    return result;
}

void ecs_modified_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_stage_t *stage = ecs_get_stage(&world);

    if (ecs_defer_modified(world, stage, entity, component)) {
        return;
    }
    
    ecs_defer_flush(world, stage);
}

static
ecs_entity_t assign_ptr_w_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    size_t size,
    void * ptr,
    bool is_move,
    bool notify)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    (void)notify;

    if (!entity) {
        entity = ecs_new_id(world);
    }

    if (ecs_defer_set(world, stage, EcsOpSet, entity, component, 
        ecs_from_size_t(size), ptr, NULL, NULL))
    {
        return entity;
    }

    ecs_entity_info_t info;

    void *dst = get_mutable(world, entity, component, &info, NULL);

    ecs_assert(dst != NULL, ECS_INTERNAL_ERROR);

    if (ptr) {
        ecs_entity_t real_id = ecs_component_id_from_id(world, component);
        ecs_lifecycle_t *cdata = get_lifecycle(world, real_id);
        if (cdata) {
            if (is_move) {
                ecs_move_t move = cdata->move;
                if (move) {
                    move(dst, ptr, size, 1);
                } else {
                    ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
                }
            } else {
                ecs_copy_t copy = cdata->copy;
                if (copy) {
                    copy(dst, ptr, size, 1);
                } else {
                    ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
                }
            }
        } else {
            ecs_os_memcpy(dst, ptr, ecs_from_size_t(size));
        }
    } else {
        memset(dst, 0, size);
    }

    ecs_defer_flush(world, stage);

    return entity;
}

ecs_entity_t ecs_set_ptr_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component,
    const void *ptr,
    size_t size)
{
    /* Safe to cast away const: function won't modify if move arg is false */
    return assign_ptr_w_id(
        world, entity, component, size, (void*)ptr, false, true);
}

bool ecs_has_id(
    const ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_type_t type = ecs_get_type(world, entity);
    return ecs_type_has_entity(world, type, component);
}

const char* ecs_get_name(
    const ecs_world_t *world,
    ecs_entity_t entity)
{
    const EcsName *id = ecs_get(world, entity, EcsName);

    if (id) {
        return id->value;
    } else {
        return NULL;
    }
}

bool ecs_is_alive(
    const ecs_world_t *world,
    ecs_entity_t e)
{
    return ecs_eis_is_alive(world, e);
}

bool ecs_exists(
    const ecs_world_t *world,
    ecs_entity_t e)
{
    return ecs_eis_exists(world, e);
}

ecs_type_t ecs_get_type(
    const ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER);
    ecs_record_t *record = NULL;

    record = ecs_eis_get(world, entity);

    ecs_table_t *table;
    if (record && (table = record->table)) {
        return table->type;
    }
    
    return NULL;
}

void ecs_defer_begin(
    ecs_world_t *world)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    
    if (world->in_progress) {
        ecs_stage_defer_begin(world, stage);
    } else {
        ecs_defer_none(world, stage);
    }
}

void ecs_defer_end(
    ecs_world_t *world)
{
    ecs_stage_t *stage = ecs_get_stage(&world);
    
    if (world->in_progress) {
        ecs_stage_defer_end(world, stage);
    } else {
        ecs_defer_flush(world, stage);
    }
}

static
void discard_op(
    ecs_defer_t * op)
{
    void *value = op->is._1.value;
    if (value) {
        ecs_os_free(value);
    }

    ecs_entity_t *components = op->components.array;
    if (components) {
        ecs_os_free(components);
    }
}

/* Leave safe section. Run all deferred commands. */
void ecs_defer_flush(
    ecs_world_t * world,
    ecs_stage_t * stage)
{
    if (!--stage->defer) {
        ecs_vector_t *defer_queue = stage->defer_queue;
        stage->defer_queue = NULL;
        if (defer_queue) {
            ecs_defer_t *ops = ecs_vector_first(defer_queue, ecs_defer_t);
            int32_t i, count = ecs_vector_count(defer_queue);
            
            for (i = 0; i < count; i ++) {
                ecs_defer_t *op = &ops[i];
                ecs_entity_t e = op->is._1.entity;

                /* If entity is no longer alive, this could be because the queue
                 * contained both a delete and a subsequent add/remove/set which
                 * should be ignored. */
                if (e && !ecs_is_alive(world, e) && ecs_eis_exists(world, e)) {
                    ecs_assert(op->kind != EcsOpNew, ECS_INTERNAL_ERROR);
                    discard_op(op);
                    continue;
                }

                if (op->components.count == 1) {
                    op->components.array = &op->component;
                }

                switch(op->kind) {
                case EcsOpNew:

                    /* Fallthrough */
                case EcsOpAdd:
                    ecs_add_id(world, e, op->component);
                    break;
                case EcsOpRemove:
                    ecs_remove_id(world, e, op->component);
                    break;
                case EcsOpSet:
                    assign_ptr_w_id(world, e, 
                        op->component, ecs_to_size_t(op->is._1.size), 
                        op->is._1.value, true, true);
                    break;
                case EcsOpMut:
                    assign_ptr_w_id(world, e, 
                        op->component, ecs_to_size_t(op->is._1.size), 
                        op->is._1.value, true, false);
                    break;
                case EcsOpModified:
                    ecs_modified_id(world, e, op->component);
                    break;
                case EcsOpDelete: {
                    ecs_delete(world, e);
                    break;
                }
                case EcsOpClear:
                    ecs_clear(world, e);
                    break;
                }

                if (op->components.count > 1) {
                    ecs_os_free(op->components.array);
                }

                if (op->is._1.value) {
                    ecs_os_free(op->is._1.value);
                }
            };

            if (defer_queue != stage->defer_merge_queue) {
                ecs_vector_free(defer_queue);
            }
        }
    }
}

static
size_t append_to_str(
    char **buffer,
    const char *str,
    size_t bytes_left,
    size_t *required)
{
    char *ptr = *buffer;

    size_t len = strlen(str);
    size_t to_write;
    if (bytes_left < len) {
        to_write = bytes_left;
        bytes_left = 0;
    } else {
        to_write = len;
        bytes_left -= len;
    }
    
    if (to_write) {
        ecs_os_memcpy(ptr, str, to_write);
    }

    (*required) += len;
    (*buffer) += to_write;

    return bytes_left;
}

static
size_t append_entity_to_str(
    ecs_world_t *world,
    char **ptr,
    ecs_entity_t entity,
    size_t bytes_left,
    size_t *required)
{
    const char *name = ecs_get_name(world, entity);
    if (name) {
        bytes_left = append_to_str(
            ptr, name, bytes_left, required);
    } else {
        char buf[14];
        ecs_os_sprintf(buf, "%u", (uint32_t)entity);
        bytes_left = append_to_str(
            ptr, buf, bytes_left, required);
    }
    return bytes_left;
}

size_t ecs_entity_str(
    ecs_world_t *world,
    ecs_entity_t entity,
    char *buffer,
    size_t buffer_len)
{
    char *ptr = buffer;
    size_t bytes_left = buffer_len - 1, required = 0;
    if (entity & ECS_ROLE) {
        bytes_left = append_entity_to_str(world, &ptr, ecs_get_role(entity), 
            bytes_left, &required);
        bytes_left = append_to_str(&ptr, "|", bytes_left, &required);
        entity = entity & ECS_ENTITY_MASK;
    }

    bytes_left = append_entity_to_str(world, &ptr, entity, bytes_left, 
        &required);

    ptr[0] = '\0';
    return required;
}

ecs_entity_t ecs_find_in_type(
    ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t component,
    ecs_entity_t role)
{
    ecs_vector_each(type, ecs_entity_t, c_ptr, {
        ecs_entity_t c = *c_ptr;

        if (role) {
            if (!ecs_has_role(c, role)) {
                continue;
            }
        }

        ecs_entity_t e = c & ECS_ENTITY_MASK;

        if (component) {
           ecs_type_t component_type = ecs_get_type(world, e);
           if (!ecs_type_has_entity(world, component_type, component)) {
               continue;
           }
        }

        return e;
    });

    return 0;
}

ecs_entity_t ecs_get_parent_w_id(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_t component)
{
    ecs_type_t type = ecs_get_type(world, entity);    
    ecs_entity_t parent = ecs_find_in_type(world, type, component, EcsScope);
    return parent;
}

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


/** Resize the vector buffer */
static
ecs_vector_t* resize(
    ecs_vector_t *vector,
    int16_t offset,
    int32_t size)
{
    ecs_vector_t *result = ecs_os_realloc(vector, offset + size);
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);
    return result;
}

/* -- Public functions -- */

ecs_vector_t* _ecs_vector_new(
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    ecs_assert(elem_size != 0, ECS_INTERNAL_ERROR);
    
    ecs_vector_t *result =
        ecs_os_malloc(offset + elem_size * elem_count);
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);

    result->count = 0;
    result->size = elem_count;
#ifndef NDEBUG
    result->elem_size = elem_size;
#endif
    return result;
}

ecs_vector_t* _ecs_vector_from_array(
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count,
    void *array)
{
    ecs_assert(elem_size != 0, ECS_INTERNAL_ERROR);
    
    ecs_vector_t *result =
        ecs_os_malloc(offset + elem_size * elem_count);
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);

    ecs_os_memcpy(ECS_OFFSET(result, offset), array, elem_size * elem_count);

    result->count = elem_count;
    result->size = elem_count;
#ifndef NDEBUG
    result->elem_size = elem_size;
#endif
    return result;   
}

void ecs_vector_free(
    ecs_vector_t *vector)
{
    ecs_os_free(vector);
}

void ecs_vector_clear(
    ecs_vector_t *vector)
{
    if (vector) {
        vector->count = 0;
    }
}

void ecs_vector_assert_size(
    ecs_vector_t *vector,
    ecs_size_t elem_size)
{
    if (vector) {
        ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
    }
}

void* _ecs_vector_addn(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    ecs_assert(array_inout != NULL, ECS_INTERNAL_ERROR);
    
    if (elem_count == 1) {
        return _ecs_vector_add(array_inout, elem_size, offset);
    }
    
    ecs_vector_t *vector = *array_inout;
    if (!vector) {
        vector = _ecs_vector_new(elem_size, offset, 1);
        *array_inout = vector;
    }

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);

    int32_t max_count = vector->size;
    int32_t old_count = vector->count;
    int32_t new_count = old_count + elem_count;

    if ((new_count - 1) >= max_count) {
        if (!max_count) {
            max_count = elem_count;
        } else {
            while (max_count < new_count) {
                max_count *= 2;
            }
        }

        vector = resize(vector, offset, max_count * elem_size);
        vector->size = max_count;
        *array_inout = vector;
    }

    vector->count = new_count;

    return ECS_OFFSET(vector, offset + elem_size * old_count);
}

void* _ecs_vector_add(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset)
{
    ecs_vector_t *vector = *array_inout;
    int32_t count, size;

    if (vector) {
        ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
        count = vector->count;
        size = vector->size;

        if (count >= size) {
            size *= 2;
            if (!size) {
                size = 2;
            }
            vector = resize(vector, offset, size * elem_size);
            *array_inout = vector;
            vector->size = size;
        }

        vector->count = count + 1;
        return ECS_OFFSET(vector, offset + elem_size * count);
    }

    vector = _ecs_vector_new(elem_size, offset, 2);
    *array_inout = vector;
    vector->count = 1;
    vector->size = 2;
    return ECS_OFFSET(vector, offset);
}

void ecs_vector_remove_last(
    ecs_vector_t *vector)
{
    if (vector && vector->count) vector->count --;
}

bool _ecs_vector_pop(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    void *value)
{
    if (!vector) {
        return false;
    }

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);

    int32_t count = vector->count;
    if (!count) {
        return false;
    }

    void *elem = ECS_OFFSET(vector, offset + (count - 1) * elem_size);

    if (value) {
        ecs_os_memcpy(value, elem, elem_size);
    }

    ecs_vector_remove_last(vector);

    return true;
}

int32_t _ecs_vector_remove(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t index)
{
    if (!vector) {
        return 0;
    }

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
    
    int32_t count = vector->count;
    void *buffer = ECS_OFFSET(vector, offset);
    void *elem = ECS_OFFSET(buffer, index * elem_size);

    ecs_assert(index < count, ECS_INVALID_PARAMETER);

    count --;
    if (index != count) {
        void *last_elem = ECS_OFFSET(buffer, elem_size * count);
        ecs_os_memcpy(elem, last_elem, elem_size);
    }

    vector->count = count;

    return count;
}

void _ecs_vector_reclaim(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset)
{
    ecs_vector_t *vector = *array_inout;

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
    
    int32_t size = vector->size;
    int32_t count = vector->count;

    if (count < size) {
        size = count;
        vector = resize(vector, offset, size * elem_size);
        vector->size = size;
        *array_inout = vector;
    }
}

int32_t ecs_vector_count(
    const ecs_vector_t *vector)
{
    if (!vector) {
        return 0;
    }
    return vector->count;
}

int32_t ecs_vector_size(
    const ecs_vector_t *vector)
{
    if (!vector) {
        return 0;
    }
    return vector->size;
}

int32_t _ecs_vector_set_size(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    ecs_vector_t *vector = *array_inout;

    if (!vector) {
        *array_inout = _ecs_vector_new(elem_size, offset, elem_count);
        return elem_count;
    } else {
        ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);

        int32_t result = vector->size;

        if (elem_count < vector->count) {
            elem_count = vector->count;
        }

        if (result < elem_count) {
            vector = resize(vector, offset, elem_count * elem_size);
            vector->size = elem_count;
            *array_inout = vector;
            result = elem_count;
        }

        return result;
    }
}

int32_t _ecs_vector_grow(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    int32_t current = ecs_vector_count(*array_inout);
    return _ecs_vector_set_size(array_inout, elem_size, offset, current + elem_count);
}

int32_t _ecs_vector_set_count(
    ecs_vector_t **array_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    if (!*array_inout) {
        *array_inout = _ecs_vector_new(elem_size, offset, elem_count);
    }

    ecs_assert((*array_inout)->elem_size == elem_size, ECS_INTERNAL_ERROR);

    (*array_inout)->count = elem_count;

    ecs_size_t size = _ecs_vector_set_size(
        array_inout, elem_size, offset, ecs_next_pow_of_2(elem_count));
    return size;
}

void* _ecs_vector_first(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset)
{
    (void)elem_size;

    ecs_assert(!vector || vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
    if (vector && vector->size) {
        return ECS_OFFSET(vector, offset);
    } else {
        return NULL;
    }
}

void* _ecs_vector_get(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t index)
{
    if (!vector) {
        return NULL;
    }
    
    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);    
    ecs_assert(index >= 0, ECS_INTERNAL_ERROR);

    int32_t count = vector->count;

    if (index >= count) {
        return NULL;
    }

    return ECS_OFFSET(vector, offset + elem_size * index);
}

void* _ecs_vector_last(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset)
{
    if (vector) {
        ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);
        int32_t count = vector->count;
        if (!count) {
            return NULL;
        } else {
            return ECS_OFFSET(vector, offset + elem_size * (count - 1));
        }
    } else {
        return NULL;
    }
}

int32_t _ecs_vector_set_min_size(
    ecs_vector_t **vector_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    if (!*vector_inout || (*vector_inout)->size < elem_count) {
        return _ecs_vector_set_size(vector_inout, elem_size, offset, elem_count);
    } else {
        return (*vector_inout)->size;
    }
}

int32_t _ecs_vector_set_min_count(
    ecs_vector_t **vector_inout,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t elem_count)
{
    _ecs_vector_set_min_size(vector_inout, elem_size, offset, elem_count);

    ecs_vector_t *v = *vector_inout;
    if (v && v->count < elem_count) {
        v->count = elem_count;
    }

    return v->count;
}

void _ecs_vector_sort(
    ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    ecs_comparator_t compare_action)
{
    if (!vector) {
        return;
    }

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);    

    int32_t count = vector->count;
    void *buffer = ECS_OFFSET(vector, offset);

    if (count > 1) {
        qsort(buffer, (size_t)count, (size_t)elem_size, compare_action);
    }
}

void _ecs_vector_memory(
    const ecs_vector_t *vector,
    ecs_size_t elem_size,
    int16_t offset,
    int32_t *allocd,
    int32_t *used)
{
    if (!vector) {
        return;
    }

    ecs_assert(vector->elem_size == elem_size, ECS_INTERNAL_ERROR);

    if (allocd) {
        *allocd += vector->size * elem_size + offset;
    }
    if (used) {
        *used += vector->count * elem_size;
    }
}

ecs_vector_t* _ecs_vector_copy(
    const ecs_vector_t *src,
    ecs_size_t elem_size,
    int16_t offset)
{
    if (!src) {
        return NULL;
    }

    ecs_vector_t *dst = _ecs_vector_new(elem_size, offset, src->size);
    ecs_os_memcpy(dst, src, offset + elem_size * src->count);
    return dst;
}

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


#define CHUNK_COUNT (4096)
#define CHUNK(index) ((int32_t)index >> 12)
#define OFFSET(index) ((int32_t)index & 0xFFF)
#define DATA(array, size, offset) (ECS_OFFSET(array, size * offset))

typedef struct chunk_t {
    int32_t *sparse;            /* Sparse array with indices to dense array */
    void *data;                 /* Store data in sparse array to reduce  
                                 * indirection and provide stable pointers. */
} chunk_t;

struct ecs_sparse_t {
    ecs_vector_t *dense;        /* Dense array with indices to sparse array. The
                                 * dense array stores both alive and not alive
                                 * sparse indices. The 'count' member keeps
                                 * track of which indices are alive. */

    ecs_vector_t *chunks;       /* Chunks with sparse arrays & data */
    ecs_size_t size;            /* Element size */
    int32_t count;              /* Number of alive entries */
    uint64_t max_id_local;      /* Local max index (if no global is set) */
    uint64_t *max_id;           /* Maximum issued sparse index */
};

struct ecs_dense_t {
    ecs_sparse_t sparse;
    ecs_vector_t *data;
    int16_t size;
    int16_t alignment;
};

static
chunk_t* chunk_new(
    ecs_sparse_t *sparse,
    int32_t chunk_index)
{
    int32_t count = ecs_vector_count(sparse->chunks);
    chunk_t *chunks;

    if (count <= chunk_index) {
        ecs_vector_set_count(&sparse->chunks, chunk_t, chunk_index + 1);
        chunks = ecs_vector_first(sparse->chunks, chunk_t);
        ecs_os_memset(&chunks[count], 0, (1 + chunk_index - count) * ECS_SIZEOF(chunk_t));
    } else {
        chunks = ecs_vector_first(sparse->chunks, chunk_t);
    }

    ecs_assert(chunks != NULL, ECS_INTERNAL_ERROR);

    chunk_t *result = &chunks[chunk_index];
    ecs_assert(result->sparse == NULL, ECS_INTERNAL_ERROR);
    ecs_assert(result->data == NULL, ECS_INTERNAL_ERROR);

    /* Initialize sparse array with zero's, as zero is used to indicate that the
     * sparse element has not been paired with a dense element. Use zero
     * as this means we can take advantage of calloc having a possibly better 
     * performance than malloc + memset. */
    result->sparse = ecs_os_calloc(ECS_SIZEOF(int32_t) * CHUNK_COUNT);
    ecs_assert(result->sparse != NULL, ECS_OUT_OF_MEMORY);

    /* Initialize the data array with zero's to guarantee that data is 
     * always initialized. When an entry is removed, data is reset back to
     * zero. Initialize now, as this can take advantage of calloc. */
    result->data = ecs_os_calloc(sparse->size * CHUNK_COUNT);
    ecs_assert(result->data != NULL, ECS_OUT_OF_MEMORY);

    return result;
}

static
void chunk_free(
    chunk_t *chunk)
{
    ecs_os_free(chunk->sparse);
    ecs_os_free(chunk->data);
}

static
chunk_t* get_chunk(
    const ecs_sparse_t *sparse,
    int32_t chunk_index)
{
    /* If chunk_index is below zero, application used an invalid entity id */
    ecs_assert(chunk_index >= 0, ECS_INVALID_PARAMETER);
    chunk_t *result = ecs_vector_get(sparse->chunks, chunk_t, chunk_index);
    if (result && !result->sparse) {
        return NULL;
    }

    return result;
}

static
chunk_t* get_or_create_chunk(
    ecs_sparse_t *sparse,
    int32_t chunk_index)
{
    chunk_t *chunk = get_chunk(sparse, chunk_index);
    if (chunk) {
        return chunk;
    }

    return chunk_new(sparse, chunk_index);
}

static
void grow_dense(
    ecs_sparse_t *sparse)
{
    ecs_vector_add(&sparse->dense, uint64_t);
}

static
uint64_t strip_generation(
    uint64_t *index_out)
{
    uint64_t index = *index_out;
    uint64_t gen = index & ECS_GENERATION_MASK;
    *index_out -= gen;
    return gen;
}

static
void assign_index(
    chunk_t * chunk, 
    uint64_t * dense_array, 
    uint64_t index, 
    int32_t dense)
{
    chunk->sparse[OFFSET(index)] = dense;
    dense_array[dense] = index;
}

static
uint64_t inc_gen(
    uint64_t index)
{
    return ECS_GENERATION_INC(index);
}

static
uint64_t inc_id(
    ecs_sparse_t *sparse)
{
    return ++ (sparse->max_id[0]);
}

static
uint64_t get_id(
    const ecs_sparse_t *sparse)
{
    return sparse->max_id[0];
}

static
void set_id(
    ecs_sparse_t *sparse,
    uint64_t value)
{
    sparse->max_id[0] = value;
}

static
uint64_t create_id(
    ecs_sparse_t *sparse,
    int32_t dense)
{
    uint64_t index = inc_id(sparse);
    grow_dense(sparse);

    chunk_t *chunk = get_or_create_chunk(sparse, CHUNK(index));
    ecs_assert(chunk->sparse[OFFSET(index)] == 0, ECS_INTERNAL_ERROR);
    
    uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
    assign_index(chunk, dense_array, index, dense);
    
    return index;
}

static
uint64_t new_index(
    ecs_sparse_t *sparse)
{
    ecs_vector_t *dense = sparse->dense;
    int32_t dense_count = ecs_vector_count(dense);
    int32_t count = sparse->count ++;

    ecs_assert(count <= dense_count, ECS_INTERNAL_ERROR);

    if (count < dense_count) {
        /* If there are unused elements in the dense array, return first */
        uint64_t *dense_array = ecs_vector_first(dense, uint64_t);
        return dense_array[count];
    } else {
        return create_id(sparse, count);
    }
}

static
void* try_sparse_any(
    const ecs_sparse_t *sparse,
    uint64_t index)
{    
    strip_generation(&index);

    chunk_t *chunk = get_chunk(sparse, CHUNK(index));
    if (!chunk) {
        return NULL;
    }

    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];
    bool in_use = dense && (dense < sparse->count);
    if (!in_use) {
        return NULL;
    }

    ecs_assert(dense == chunk->sparse[offset], ECS_INTERNAL_ERROR);
    return DATA(chunk->data, sparse->size, offset);
}

static
void* try_sparse(
    const ecs_sparse_t *sparse,
    uint64_t index)
{
    chunk_t *chunk = get_chunk(sparse, CHUNK(index));
    if (!chunk) {
        return NULL;
    }

    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];
    bool in_use = dense && (dense < sparse->count);
    if (!in_use) {
        return NULL;
    }

    uint64_t gen = strip_generation(&index);
    uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
    uint64_t cur_gen = dense_array[dense] & ECS_GENERATION_MASK;

    if (cur_gen != gen) {
        return NULL;
    }

    ecs_assert(dense == chunk->sparse[offset], ECS_INTERNAL_ERROR);
    return DATA(chunk->data, sparse->size, offset);
}

static
void* get_sparse(
    const ecs_sparse_t *sparse,
    int32_t dense,
    uint64_t index)
{
    strip_generation(&index);
    chunk_t *chunk = get_chunk(sparse, CHUNK(index));
    int32_t offset = OFFSET(index);
    
    ecs_assert(chunk != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(dense == chunk->sparse[offset], ECS_INTERNAL_ERROR);

    return DATA(chunk->data, sparse->size, offset);
}

static
void swap_dense(
    ecs_sparse_t * sparse,
    chunk_t * chunk_a,
    int32_t a,
    int32_t b)
{
    ecs_assert(a != b, ECS_INTERNAL_ERROR);
    uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
    uint64_t index_a = dense_array[a];
    uint64_t index_b = dense_array[b];

    chunk_t *chunk_b = get_or_create_chunk(sparse, CHUNK(index_b));
    assign_index(chunk_a, dense_array, index_a, b);
    assign_index(chunk_b, dense_array, index_b, a);
}

static
void init_sparse(
    ecs_sparse_t *sparse,
    ecs_size_t size)
{
    ecs_assert(sparse != NULL, ECS_OUT_OF_MEMORY);
    sparse->size = size;
    sparse->max_id_local = UINT64_MAX;
    sparse->max_id = &sparse->max_id_local;

    /* Consume first value in dense array as 0 is used in the sparse array to
     * indicate that a sparse element hasn't been paired yet. */
    ecs_vector_add(&sparse->dense, uint64_t);
    sparse->count = 1;
}

ecs_sparse_t* _ecs_sparse_new(
    ecs_size_t size)
{
    ecs_sparse_t *result = ecs_os_calloc(ECS_SIZEOF(ecs_sparse_t));
    init_sparse(result, size);
    return result;
}

void ecs_sparse_set_id_source(
    ecs_sparse_t * sparse,
    uint64_t * id_source)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    sparse->max_id = id_source;
}

void ecs_sparse_clear(
    ecs_sparse_t *sparse)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);

    ecs_vector_each(sparse->chunks, chunk_t, chunk, {
        chunk_free(chunk);
    });

    ecs_vector_free(sparse->chunks);
    ecs_vector_set_count(&sparse->dense, uint64_t, 1);

    sparse->chunks = NULL;   
    sparse->count = 1;
    sparse->max_id_local = 0;
}

void ecs_sparse_free(
    ecs_sparse_t *sparse)
{
    if (sparse) {
        ecs_sparse_clear(sparse);
        ecs_vector_free(sparse->dense);
        ecs_os_free(sparse);
    }
}

uint64_t ecs_sparse_new_id(
    ecs_sparse_t *sparse)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    return new_index(sparse);
}

const uint64_t* ecs_sparse_new_ids(
    ecs_sparse_t *sparse,
    int32_t new_count)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    int32_t dense_count = ecs_vector_count(sparse->dense);
    int32_t count = sparse->count;
    int32_t remaining = dense_count - count;
    int32_t i, to_create = new_count - remaining;

    if (to_create > 0) {
        ecs_sparse_set_size(sparse, dense_count + to_create);
        uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);

        for (i = 0; i < to_create; i ++) {
            uint64_t index = create_id(sparse, count + i);
            dense_array[dense_count + i] = index;
        }
    }

    sparse->count += new_count;

    return ecs_vector_get(sparse->dense, uint64_t, count);
}

void* _ecs_sparse_add(
    ecs_sparse_t *sparse,
    ecs_size_t size)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    uint64_t index = new_index(sparse);
    chunk_t *chunk = get_chunk(sparse, CHUNK(index));
    ecs_assert(chunk != NULL, ECS_INTERNAL_ERROR);
    return DATA(chunk->data, size, OFFSET(index));
}

uint64_t ecs_sparse_last_id(
    ecs_sparse_t *sparse)
{
    ecs_assert(sparse != NULL, ECS_INTERNAL_ERROR);
    uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
    return dense_array[sparse->count - 1];
}

static
void* get_or_create(
    ecs_sparse_t *sparse,
    uint64_t index,
    uint64_t gen,
    chunk_t *chunk,
    int32_t offset,
    int32_t dense)
{
    if (dense) {
        /* Check if element is alive. If element is not alive, update indices so
         * that the first unused dense element points to the sparse element. */
        int32_t count = sparse->count;
        if (dense == count) {
            /* If dense is the next unused element in the array, simply increase
             * the count to make it part of the alive set. */
            sparse->count ++;
        } else if (dense > count) {
            /* If dense is not alive, swap it with the first unused element. */
            swap_dense(sparse, chunk, dense, count);

            /* First unused element is now last used element */
            sparse->count ++;
        } else {
            /* Dense is already alive, nothing to be done */
        }
    } else {
        /* Element is not paired yet. Must add a new element to dense array */
        grow_dense(sparse);

        ecs_vector_t *dense_vector = sparse->dense;
        uint64_t *dense_array = ecs_vector_first(dense_vector, uint64_t);
        int32_t dense_count = ecs_vector_count(dense_vector) - 1;
        int32_t count = sparse->count ++;

        /* If index is larger than max id, update max id */
        uint64_t max_id = get_id(sparse);
        if ((max_id == (uint64_t)-1) || (index >= get_id(sparse))) {
            set_id(sparse, index);
        }

        if (count < dense_count) {
            /* If there are unused elements in the list, move the first unused
             * element to the end of the list */
            uint64_t unused = dense_array[count];
            chunk_t *unused_chunk = get_or_create_chunk(sparse, CHUNK(unused));
            assign_index(unused_chunk, dense_array, unused, dense_count);
        }

        assign_index(chunk, dense_array, index, count);
        dense_array[count] |= gen;
    }

    return DATA(chunk->data, sparse->size, offset);
}

void* _ecs_sparse_get_or_create(
    ecs_sparse_t *sparse,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    ecs_assert(ecs_vector_count(sparse->dense) > 0, ECS_INTERNAL_ERROR);

    uint64_t gen = strip_generation(&index);
    chunk_t *chunk = get_or_create_chunk(sparse, CHUNK(index));
    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];

    return get_or_create(sparse, index, gen, chunk, offset, dense);
}

static
void* remove_get(
    ecs_sparse_t *sparse,
    uint64_t index,
    uint64_t gen,
    chunk_t *chunk,
    int32_t offset,
    int32_t dense)    
{
    if (dense) {
        uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
        uint64_t cur_gen = dense_array[dense] & ECS_GENERATION_MASK;
        if (gen != cur_gen) {
            /* Generation doesn't match which means that the provided entity is
             * already not alive. */
            return NULL;
        }

        /* Increase generation */
        dense_array[dense] = index | inc_gen(cur_gen);
        
        int32_t count = sparse->count;
        if (dense == (count - 1)) {
            /* If dense is the last used element, simply decrease count */
            sparse->count --;
        } else if (dense < count) {
            /* If element is alive, move it to unused elements */
            swap_dense(sparse, chunk, dense, count - 1);
            sparse->count --;
        } else {
            /* Element is not alive, nothing to be done */
            return NULL;
        }

        /* Reset memory to zero on remove */
        return DATA(chunk->data, sparse->size, offset);
    } else {
        /* Element is not paired and thus not alive, nothing to be done */
        return NULL;
    }
}

void* _ecs_sparse_remove_get(
    ecs_sparse_t *sparse,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    chunk_t *chunk = get_or_create_chunk(sparse, CHUNK(index));
    uint64_t gen = strip_generation(&index);
    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];
    return remove_get(sparse, index, gen, chunk, offset, dense);
}

void ecs_sparse_remove(
    ecs_sparse_t *sparse,
    uint64_t index)
{
    void *ptr = _ecs_sparse_remove_get(sparse, 0, index);
    if (ptr) {
        ecs_os_memset(ptr, 0, sparse->size);
    }
}

void ecs_sparse_set_generation(
    ecs_sparse_t *sparse,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    chunk_t *chunk = get_or_create_chunk(sparse, CHUNK(index));
    
    uint64_t index_w_gen = index;
    strip_generation(&index);
    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];

    if (dense) {
        /* Increase generation */
        uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
        dense_array[dense] = index_w_gen;
    } else {
        /* Element is not paired and thus not alive, nothing to be done */
    }
}

bool ecs_sparse_exists(
    ecs_sparse_t *sparse,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    chunk_t *chunk = get_or_create_chunk(sparse, CHUNK(index));
    
    strip_generation(&index);
    int32_t offset = OFFSET(index);
    int32_t dense = chunk->sparse[offset];

    return (dense != 0) && (dense < sparse->count);
}

void* _ecs_sparse_get_elem(
    const ecs_sparse_t *sparse,
    ecs_size_t size,
    int32_t dense_index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    ecs_assert(dense_index < sparse->count, ECS_INVALID_PARAMETER);

    dense_index ++;

    uint64_t *dense_array = ecs_vector_first(sparse->dense, uint64_t);
    return get_sparse(sparse, dense_index, dense_array[dense_index]);
}

bool ecs_sparse_is_alive(
    const ecs_sparse_t *sparse,
    uint64_t index)
{
    return try_sparse(sparse, index) != NULL;
}

void* _ecs_sparse_get(
    const ecs_sparse_t *sparse,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    return try_sparse(sparse, index);
}

void* _ecs_sparse_get_any(
    ecs_sparse_t *sparse,
    ecs_size_t size,
    uint64_t index)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!size || size == sparse->size, ECS_INVALID_PARAMETER);
    return try_sparse_any(sparse, index);
}

int32_t ecs_sparse_count(
    const ecs_sparse_t *sparse)
{
    if (!sparse) {
        return 0;
    }

    return sparse->count - 1;
}

int32_t ecs_sparse_size(
    const ecs_sparse_t *sparse)
{
    if (!sparse) {
        return 0;
    }

    return ecs_vector_count(sparse->dense) - 1;
}

const uint64_t* ecs_sparse_ids(
    const ecs_sparse_t *sparse)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    return &(ecs_vector_first(sparse->dense, uint64_t)[1]);
}

void ecs_sparse_set_size(
    ecs_sparse_t *sparse,
    int32_t elem_count)
{
    ecs_assert(sparse != NULL, ECS_INVALID_PARAMETER);
    ecs_vector_set_size(&sparse->dense, uint64_t, elem_count);
}

static
void sparse_copy(
    ecs_sparse_t * dst,
    const ecs_sparse_t * src)
{
    ecs_sparse_set_size(dst, ecs_sparse_size(src));
    const uint64_t *indices = ecs_sparse_ids(src);
    
    ecs_size_t size = src->size;
    int32_t i, count = src->count;

    for (i = 0; i < count - 1; i ++) {
        uint64_t index = indices[i];
        void *src_ptr = _ecs_sparse_get(src, size, index);
        void *dst_ptr = _ecs_sparse_get_or_create(dst, size, index);
        ecs_sparse_set_generation(dst, index);
        ecs_os_memcpy(dst_ptr, src_ptr, size);
    }

    set_id(dst, get_id(src));

    ecs_assert(src->count == dst->count, ECS_INTERNAL_ERROR);
}

ecs_sparse_t* ecs_sparse_copy(
    const ecs_sparse_t *src)
{
    if (!src) {
        return NULL;
    }

    ecs_sparse_t *dst = _ecs_sparse_new(src->size);
    sparse_copy(dst, src);

    return dst;
}

void ecs_sparse_restore(
    ecs_sparse_t * dst,
    const ecs_sparse_t * src)
{
    ecs_assert(dst != NULL, ECS_INVALID_PARAMETER);
    dst->count = 1;
    if (src) {
        sparse_copy(dst, src);
    }
}

void ecs_sparse_memory(
    const ecs_sparse_t *sparse,
    int32_t *allocd,
    int32_t *used)
{
    if (!sparse) {
        return;
    }

    *allocd += ECS_SIZEOF(ecs_sparse_t);

    ecs_vector_memory(sparse->chunks, chunk_t, allocd, used);
    ecs_vector_memory(sparse->dense, uint64_t, allocd, used);
    
    *allocd += ecs_vector_count(sparse->chunks) * 
        (sparse->size * CHUNK_COUNT + ECS_SIZEOF(int32_t) * CHUNK_COUNT);

    if (used) {
        *used += sparse->count * sparse->size;
    }
}

static
void ctor_init_zero(
    void *ptr,
    size_t size,
    int32_t count)
{
    ecs_os_memset(ptr, 0, ecs_from_size_t(size) * count);
}

static
void init_partition(ecs_world_t *world, ecs_partition_t *p) {    
    /* Initialize entity index */
    p->entity_index = ecs_sparse_new(ecs_record_t);
    ecs_sparse_set_id_source(p->entity_index, &world->stats.last_id);

    /* Initialize root table */
    p->tables = ecs_sparse_new(ecs_table_t);

    /* Initialize table map */
    p->table_map = ecs_map_new(ecs_table_t*, 8);

    /* Initialize one root table per stage */
    ecs_init_root_table(world, &p->root);
}

ecs_world_t *ecs_init(void) {
    ecs_os_init();

    ecs_trace_1("bootstrap");
    ecs_log_push();

    if (!ecs_os_has_heap()) {
        ecs_abort(ECS_MISSING_OS_API);
    }

    if (!ecs_os_has_threading()) {
        ecs_trace_1("threading not available");
    }

    if (!ecs_os_has_time()) {
        ecs_trace_1("time management not available");
    }

    ecs_world_t *world = ecs_os_calloc(sizeof(ecs_world_t));
    ecs_assert(world != NULL, ECS_OUT_OF_MEMORY);

    world->magic = ECS_WORLD_MAGIC;
    world->lc_hi = ecs_map_new(ecs_lifecycle_t, 0);  
    world->storages = ecs_ptree_new(ecs_store_t);
    init_partition(world, &world->store);
    ecs_stage_init(world, &world->stage);
    ecs_stage_init(world, &world->temp_stage);

    world->stage.world = world;
    world->temp_stage.world = world;

    ecs_bootstrap(world);

    ecs_log_pop();

    return world;
}

static
void fini_tables(
    ecs_world_t *world,
    ecs_partition_t *p)
{
    int32_t i, count = ecs_sparse_count(p->tables);

    for (i = 0; i < count; i ++) {
        ecs_table_t *t = ecs_sparse_get_elem(p->tables, ecs_table_t, i);
        ecs_table_free(world, t);
    }

    ecs_table_free(world, &p->root);
}

static
void fini_partition(
    ecs_world_t *world,
    ecs_partition_t *p)
{
    fini_tables(world, p);
    ecs_sparse_free(p->tables);
    ecs_sparse_free(p->entity_index);
    ecs_map_free(p->table_map);
}

static
void fini_storages(
    ecs_world_t *world)
{
    ecs_ptree_iter_t it = ecs_ptree_iter(world->storages);
    ecs_store_t *store;
    while ((store = ecs_ptree_next(&it, ecs_store_t))) {
        if (store->sparse) {
            ecs_dense_free(store->sparse);
        }
    }

    ecs_ptree_free(world->storages);
}

/* Cleanup component lifecycle callbacks & systems */
static
void fini_component_lifecycle(
    ecs_world_t *world)
{
    ecs_map_free(world->lc_hi);
}

/* Cleanup stages */
static
void fini_stages(
    ecs_world_t *world)
{
    ecs_stage_deinit(world, &world->stage);
    ecs_stage_deinit(world, &world->temp_stage);
}

/* The destroyer of worlds */
int ecs_fini(
    ecs_world_t *world)
{
    assert(world->magic == ECS_WORLD_MAGIC);
    assert(!world->in_progress);

    fini_stages(world);

    fini_partition(world, &world->store);
    fini_storages(world);

    fini_component_lifecycle(world);

    /* In case the application tries to use the memory of the freed world, this
     * will trigger an assert */
    world->magic = 0;

    ecs_increase_timer_resolution(0);

    /* End of the world */
    ecs_os_free(world);

    ecs_os_fini(); 

    return 0;
}

ecs_stage_t *ecs_get_stage(
    ecs_world_t **world_ptr)
{
    ecs_world_t *world = *world_ptr;

    ecs_assert(world->magic == ECS_WORLD_MAGIC ||
               world->magic == ECS_THREAD_MAGIC,
               ECS_INTERNAL_ERROR);

    if (world->magic == ECS_WORLD_MAGIC) {
        if (world->in_progress) {
            return &world->temp_stage;
        } else {
            return &world->stage;
        }
    } else if (world->magic == ECS_THREAD_MAGIC) {
        ecs_thread_t *thread = (ecs_thread_t*)world;
        *world_ptr = thread->world;
        return thread->stage;
    }
    
    return NULL;
}

int32_t ecs_get_thread_index(
    ecs_world_t *world)
{
    if (world->magic == ECS_THREAD_MAGIC) {
        ecs_thread_t *thr = (ecs_thread_t*)world;
        return thr->stage->id;
    } else if (world->magic == ECS_WORLD_MAGIC) {
        return 0;
    } else {
        ecs_abort(ECS_INTERNAL_ERROR);
    }
}

void ecs_notify_tables(
    ecs_world_t *world,
    ecs_table_event_t *event)
{
    ecs_sparse_t *tables = world->store.tables;
    int32_t i, count = ecs_sparse_count(tables);

    for (i = 0; i < count; i ++) {
        ecs_table_t *table = ecs_sparse_get_elem(tables, ecs_table_t, i);
        ecs_table_notify(world, table, event);
    }
}

void ecs_set_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component,
    ecs_lifecycle_t *lifecycle)
{
#ifndef NDEBUG
    const EcsComponent *component_ptr = ecs_get(world, component, EcsComponent);

    /* Cannot register lifecycle actions for things that aren't a component */
    ecs_assert(component_ptr != NULL, ECS_INVALID_PARAMETER);

    /* Cannot register lifecycle actions for components with size 0 */
    ecs_assert(component_ptr->size != 0, ECS_INVALID_PARAMETER);
#endif

    ecs_lifecycle_t *lc = ecs_get_or_create_lifecycle(world, component);
    ecs_assert(lc != NULL, ECS_INTERNAL_ERROR);

    if (lc->is_set) {
        ecs_assert(lc->ctor == lifecycle->ctor, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->dtor == lifecycle->dtor, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->copy == lifecycle->copy, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
        ecs_assert(lc->move == lifecycle->move, 
            ECS_INCONSISTENT_COMPONENT_ACTION);
    } else {
        *lc = *lifecycle;
        lc->is_set = true;

        /* If no constructor is set, invoking any of the other lifecycle actions 
         * is not safe as they will potentially access uninitialized memory. For 
         * ease of use, if no constructor is specified, set a default one that 
         * initializes the component to 0. */
        if (!lifecycle->ctor && (lifecycle->dtor || lifecycle->copy || lifecycle->move)) {
            lc->ctor = ctor_init_zero;   
        }

        ecs_notify_tables(world, &(ecs_table_event_t) {
            .kind = EcsTableComponentInfo,
            .component = component
        });
    }
}

bool ecs_component_has_actions(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_lifecycle_t *lc = ecs_get_lifecycle(world, component);
    return (lc != NULL) && lc->is_set;
}

ecs_lifecycle_t * ecs_get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{
    ecs_assert(component != 0, ECS_INTERNAL_ERROR);
    ecs_assert(!(component & ECS_ROLE), ECS_INTERNAL_ERROR);

    if (component < ECS_HI_COMPONENT_ID) {
        ecs_lifecycle_t *lc = &world->lc_lo[component];
        if (lc->is_set) {
            return lc;
        } else {
            return NULL;
        }
    } else {
        return ecs_map_get(world->lc_hi, ecs_lifecycle_t, component);
    }
}

ecs_lifecycle_t * ecs_get_or_create_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component)
{    
    ecs_lifecycle_t *lc = ecs_get_lifecycle(world, component);
    if (!lc) {
        if (component < ECS_HI_COMPONENT_ID) {
            lc = &world->lc_lo[component];
        } else {
            ecs_lifecycle_t t_info = { 0 };
            ecs_map_set(world->lc_hi, component, &t_info);
            lc = ecs_map_get(world->lc_hi, ecs_lifecycle_t, component);
            ecs_assert(lc != NULL, ECS_INTERNAL_ERROR); 
        }
    }

    return lc;
}

void ecs_dim(
    ecs_world_t *world,
    int32_t entity_count)
{
    assert(world->magic == ECS_WORLD_MAGIC);
    ecs_eis_set_size(world, entity_count + ECS_HI_COMPONENT_ID);
}

ecs_entity_t ecs_set_scope(
    ecs_world_t *world,
    ecs_entity_t scope)
{
    ecs_stage_t *stage = ecs_get_stage(&world);

    ecs_entity_t e = ecs_role(EcsScope, scope);

    ecs_entity_t cur = stage->scope;
    stage->scope = scope;

    if (scope) {
        stage->scope_table = ecs_table_traverse_add(
            world, &world->store.root, e);
    } else {
        stage->scope_table = &world->store.root;
    }

    return cur;
}

ecs_entity_t ecs_get_scope(
    const ecs_world_t *world)
{
    /* Cast is safe, ecs_get_stage doesn't modify the world */
    ecs_stage_t *stage = ecs_get_stage((ecs_world_t**)&world);
    return stage->scope;
}

void ecs_make_sparse(
    ecs_world_t *world,
    ecs_entity_t id)
{
    ecs_store_t *store = ecs_ptree_ensure(
        world->storages, ecs_store_t, id);

    if (!store->sparse) {
        const EcsComponent *ptr = ecs_get(world, id, EcsComponent);
        if (ptr) {
            store->sparse = _ecs_dense_new(ptr->size, ptr->alignment);
        } else {
            store->sparse = _ecs_dense_new(0, 0);
        }
    } 
}

#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

/*
-------------------------------------------------------------------------------
lookup3.c, by Bob Jenkins, May 2006, Public Domain.
  http://burtleburtle.net/bob/c/lookup3.c
-------------------------------------------------------------------------------
*/

#ifdef _MSC_VER
//FIXME
#else
#include <sys/param.h>  /* attempt to define endianness */
#endif
#ifdef linux
# include <endian.h>    /* attempt to define endianness */
#endif

/*
 * My best guess at if you are big-endian or little-endian.  This may
 * need adjustment.
 */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
     __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define HASH_LITTLE_ENDIAN 1
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN) || \
      (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define HASH_LITTLE_ENDIAN 0
#else
# define HASH_LITTLE_ENDIAN 0
#endif

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/*
-------------------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.

This is reversible, so any information in (a,b,c) before mix() is
still in (a,b,c) after mix().

If four pairs of (a,b,c) inputs are run through mix(), or through
mix() in reverse, there are at least 32 bits of the output that
are sometimes the same for one pair and different for another pair.
This was tested for:
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
satisfy this are
    4  6  8 16 19  4
    9 15  3 18 27 15
   14  9  3  7 17  3
Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
for "differ" defined as + with a one-bit base and a two-bit delta.  I
used http://burtleburtle.net/bob/hash/avalanche.html to choose 
the operations, constants, and arrangements of the variables.

This does not achieve avalanche.  There are input bits of (a,b,c)
that fail to affect some output bits of (a,b,c), especially of a.  The
most thoroughly mixed value is c, but it doesn't really even achieve
avalanche in c.

This allows some parallelism.  Read-after-writes are good at doubling
the number of bits affected, so the goal of mixing pulls in the opposite
direction as the goal of parallelism.  I did what I could.  Rotates
seem to cost as much as shifts on every machine I could lay my hands
on, and rotates are much kinder to the top and bottom bits, so I used
rotates.
-------------------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

/*
-------------------------------------------------------------------------------
final -- final mixing of 3 32-bit values (a,b,c) into c

Pairs of (a,b,c) values differing in only a few bits will usually
produce values of c that look totally different.  This was tested for
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

These constants passed:
 14 11 25 16 4 14 24
 12 14 25 16 4 14 24
and these came close:
  4  8 15 26 3 22 24
 10  8 15 26 3 22 24
 11  8 15 26 3 22 24
-------------------------------------------------------------------------------
*/
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}


/*
 * hashlittle2: return 2 32-bit hash values
 *
 * This is identical to hashlittle(), except it returns two 32-bit hash
 * values instead of just one.  This is good enough for hash table
 * lookup with 2^^64 buckets, or if you want a second hash if you're not
 * happy with the first, or if you want a probably-unique 64-bit ID for
 * the key.  *pc is better mixed than *pb, so use *pc first.  If you want
 * a 64-bit value do something like "*pc + (((uint64_t)*pb)<<32)".
 */
static
void hashlittle2( 
  const void *key,       /* the key to hash */
  size_t      length,    /* length of the key */
  uint32_t   *pc,        /* IN: primary initval, OUT: primary hash */
  uint32_t   *pb)        /* IN: secondary initval, OUT: secondary hash */
{
  uint32_t a,b,c;                                          /* internal state */
  union { const void *ptr; size_t i; } u;     /* needed for Mac Powerbook G4 */

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + ((uint32_t)length) + *pc;
  c += *pb;

  u.ptr = key;
  if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
    const uint32_t *k = (const uint32_t *)key;         /* read 32-bit chunks */
    const uint8_t  *k8;
    (void)k8;

    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      mix(a,b,c);
      length -= 12;
      k += 3;
    }

    /*----------------------------- handle the last (probably partial) block */
    /* 
     * "k[2]&0xffffff" actually reads beyond the end of the string, but
     * then masks off the part it's not allowed to read.  Because the
     * string is aligned, the masked-off tail is in the same word as the
     * rest of the string.  Every machine with memory protection I've seen
     * does it on word boundaries, so is OK with this.  But VALGRIND will
     * still catch it and complain.  The masking trick does make the hash
     * noticably faster for short strings (like English words).
     */
#ifndef VALGRIND

    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
    case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
    case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
    case 6 : b+=k[1]&0xffff; a+=k[0]; break;
    case 5 : b+=k[1]&0xff; a+=k[0]; break;
    case 4 : a+=k[0]; break;
    case 3 : a+=k[0]&0xffffff; break;
    case 2 : a+=k[0]&0xffff; break;
    case 1 : a+=k[0]&0xff; break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

#else /* make valgrind happy */

    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=((uint32_t)k8[10])<<16;  /* fall through */
    case 10: c+=((uint32_t)k8[9])<<8;    /* fall through */
    case 9 : c+=k8[8];                   /* fall through */
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=((uint32_t)k8[6])<<16;   /* fall through */
    case 6 : b+=((uint32_t)k8[5])<<8;    /* fall through */
    case 5 : b+=k8[4];                   /* fall through */
    case 4 : a+=k[0]; break;
    case 3 : a+=((uint32_t)k8[2])<<16;   /* fall through */
    case 2 : a+=((uint32_t)k8[1])<<8;    /* fall through */
    case 1 : a+=k8[0]; break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

#endif /* !valgrind */

  } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
    const uint16_t *k = (const uint16_t *)key;         /* read 16-bit chunks */
    const uint8_t  *k8;

    /*--------------- all but last block: aligned reads and different mixing */
    while (length > 12)
    {
      a += k[0] + (((uint32_t)k[1])<<16);
      b += k[2] + (((uint32_t)k[3])<<16);
      c += k[4] + (((uint32_t)k[5])<<16);
      mix(a,b,c);
      length -= 12;
      k += 6;
    }

    /*----------------------------- handle the last (probably partial) block */
    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[4]+(((uint32_t)k[5])<<16);
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 11: c+=((uint32_t)k8[10])<<16;     /* fall through */
    case 10: c+=k[4];
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 9 : c+=k8[8];                      /* fall through */
    case 8 : b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 7 : b+=((uint32_t)k8[6])<<16;      /* fall through */
    case 6 : b+=k[2];
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 5 : b+=k8[4];                      /* fall through */
    case 4 : a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 3 : a+=((uint32_t)k8[2])<<16;      /* fall through */
    case 2 : a+=k[0];
             break;
    case 1 : a+=k8[0];
             break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

  } else {                        /* need to read the key one byte at a time */
    const uint8_t *k = (const uint8_t *)key;

    /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      a += ((uint32_t)k[1])<<8;
      a += ((uint32_t)k[2])<<16;
      a += ((uint32_t)k[3])<<24;
      b += k[4];
      b += ((uint32_t)k[5])<<8;
      b += ((uint32_t)k[6])<<16;
      b += ((uint32_t)k[7])<<24;
      c += k[8];
      c += ((uint32_t)k[9])<<8;
      c += ((uint32_t)k[10])<<16;
      c += ((uint32_t)k[11])<<24;
      mix(a,b,c);
      length -= 12;
      k += 12;
    }

    /*-------------------------------- last block: affect all 32 bits of (c) */
    switch(length)                   /* all the case statements fall through */
    {
    case 12: c+=((uint32_t)k[11])<<24;
    case 11: c+=((uint32_t)k[10])<<16;
    case 10: c+=((uint32_t)k[9])<<8;
    case 9 : c+=k[8];
    case 8 : b+=((uint32_t)k[7])<<24;
    case 7 : b+=((uint32_t)k[6])<<16;
    case 6 : b+=((uint32_t)k[5])<<8;
    case 5 : b+=k[4];
    case 4 : a+=((uint32_t)k[3])<<24;
    case 3 : a+=((uint32_t)k[2])<<16;
    case 2 : a+=((uint32_t)k[1])<<8;
    case 1 : a+=k[0];
             break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }
  }

  final(a,b,c);
  *pc=c; *pb=b;
}

void ecs_hash(
    const void *data,
    ecs_size_t length,
    uint64_t *result)
{
    uint32_t h_1 = 0;
    uint32_t h_2 = 0;

    hashlittle2(
        data,
        ecs_to_size_t(length),
        &h_1,
        &h_2);

    *result = h_1 | ((uint64_t)h_2 << 32);
}

#define PAGE_SIZE (65536)

typedef struct addr_t {
    uint16_t value[4];
} addr_t;

typedef struct array_t {
    void *data;
    uint16_t offset;
    int32_t length;
} array_t;

typedef struct page_t {
    array_t data;
    array_t pages;
} page_t;

struct ecs_ptree_t {
    page_t root;
    void *first_65k;
    uint16_t min_65k;
    uint16_t max_65k;
    uint8_t elem_size;
};

static
addr_t to_addr(
    uint64_t id) 
{
    union {
        addr_t addr;
        uint64_t id;
    } u = {.id = id};
    return u.addr;
}

static
void* array_ensure(
    array_t *array,
    ecs_size_t elem_size,
    uint16_t index)
{
    int32_t offset = (int32_t)array->offset;
    int32_t length = array->length;

    ecs_assert(offset >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(length >= 0, ECS_INTERNAL_ERROR);

    if (!length) {
        array->length = 1;
        array->offset = index;
        array->data = ecs_os_calloc(elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY);
        return array->data; 

    } else if (index < offset) {
        int32_t new_offset = ecs_next_pow_of_2(index) >> 1;
        int32_t dif_offset = offset - new_offset;
        int32_t new_length = ecs_next_pow_of_2(length + dif_offset);
        
        if ((new_offset + new_length) > PAGE_SIZE) {
            new_offset = 0;
            new_length = PAGE_SIZE;
            dif_offset = offset;
        }

        ecs_assert(length != new_length, ECS_INTERNAL_ERROR);
        array->data = ecs_os_realloc(array->data, new_length * elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY);

        ecs_os_memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
        ecs_os_memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
            length * elem_size);
        ecs_os_memset(array->data, 0, dif_offset * elem_size);            

        ecs_assert(new_offset >= 0, ECS_INTERNAL_ERROR);
        ecs_assert(new_offset < 65536, ECS_INTERNAL_ERROR);

        array->offset = (uint16_t)new_offset;
        array->length = new_length;

    } else if (index >= (offset + length)) {
        int32_t new_length = ecs_next_pow_of_2(index + offset + 1);
        if ((new_length + offset) > PAGE_SIZE) {
            int32_t new_offset = ecs_next_pow_of_2(offset - ((new_length + offset) - PAGE_SIZE)) >> 1;
            int32_t dif_offset = offset - new_offset;
            new_length = ecs_next_pow_of_2(new_offset + index);
            
            ecs_assert(length != new_length, ECS_INTERNAL_ERROR);
            array->data = ecs_os_realloc(array->data, new_length * elem_size);
            ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY);

            ecs_os_memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
            ecs_os_memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
                length * elem_size);
            ecs_os_memset(array->data, 0, dif_offset * elem_size);

            ecs_assert(new_offset >= 0, ECS_INTERNAL_ERROR);
            ecs_assert(new_offset < 65536, ECS_INTERNAL_ERROR);

            array->offset = (uint16_t)new_offset;
        } else {
            ecs_assert(length != new_length, ECS_INTERNAL_ERROR);
            array->data = ecs_os_realloc(array->data, new_length * elem_size);
            ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY);

            ecs_os_memset(ECS_OFFSET(array->data, length * elem_size), 0, 
                (new_length - length) * elem_size);
        }

        array->length = new_length;
    }

    ecs_assert((array->offset + array->length) <= PAGE_SIZE, 
        ECS_INTERNAL_ERROR);
    
    return ECS_OFFSET(array->data, (index - array->offset) * elem_size);
}

static
void* array_get(
    array_t *array,
    ecs_size_t elem_size,
    int32_t index)
{
    uint16_t offset = array->offset;
    int32_t length = array->length;

    if (index < offset) {
        return NULL;
    }

    index -= offset;
    if (index >= length) {
        return NULL;
    }

    return ECS_OFFSET(array->data, index * elem_size);
}

static
page_t* get_page(
    page_t *p,
    uint16_t *addr,
    int count) 
{
    int32_t i;
    for (i = count; i > 0; i --) {
        p = array_get(&p->pages, ECS_SIZEOF(page_t), addr[i]);
        if (!p) {
            return NULL;
        }
    }

    return p;
}

static
page_t* get_or_create_page(
    page_t *p,
    uint16_t *addr,
    int count)
{
    int32_t i;
    for (i = count; i > 0; i --) {
        p = array_ensure(&p->pages, ECS_SIZEOF(page_t), addr[i]);
        ecs_assert(p != NULL, ECS_INTERNAL_ERROR);
    }

    return p;
}

static
int8_t page_count(
    uint64_t index)
{
    return (int8_t)(1 +
        (index > 0x00000000FFFF0000) +
        (index > 0x0000FFFF00000000));
}

static
int32_t page_free(
    ecs_ptree_t *ptree,
    page_t *p)
{
    int32_t result = sizeof(page_t);

    if (p->data.data) {
        result += p->data.length * ECS_SIZEOF(ptree->elem_size);
        ecs_os_free(p->data.data);
    }

    if (p->pages.data) {
        int32_t i;
        for (i = 0; i < p->pages.length; i ++) {
            result += page_free(ptree, array_get(&p->pages, ECS_SIZEOF(page_t), 
                i + p->pages.offset));
        }

        ecs_os_free(p->pages.data);
    }

    return result;
}

static
uint64_t index_from_it(
    ecs_ptree_iter_t *it,
    page_t *page)
{
    ecs_assert(it->cur_elem >= 0, ECS_INTERNAL_ERROR);
    
    uint64_t result = page->data.offset + (uint32_t)it->cur_elem;
    int8_t sp = it->sp;

    int8_t i;
    for (i = 0; i <= sp; i ++) {
        result += (uint64_t)((it->cur_page[i] + 
            ((page_t*)it->frames[i])->pages.offset)) << (16 * (1 + sp - i));
    }

    return result;
}

ecs_ptree_t* _ecs_ptiny_new(
    ecs_size_t elem_size)
{
    ecs_ptree_t *result = ecs_os_calloc(sizeof(ecs_ptree_t));
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);
    ecs_assert(elem_size < 256, ECS_INVALID_PARAMETER);
    result->elem_size = (uint8_t)elem_size;
    return result;
}

ecs_ptree_t* _ecs_ptree_new(
    ecs_size_t elem_size)
{
    ecs_ptree_t *result = _ecs_ptiny_new(elem_size);
    result->first_65k = ecs_os_calloc(elem_size * 65536);
    result->min_65k = 65535;
    return result;
}

int32_t ecs_ptree_free(
    ecs_ptree_t *ptree)
{
    int32_t result = page_free(ptree, &ptree->root);
    result += ECS_SIZEOF(int) * 65536;
    result += ECS_SIZEOF(ecs_ptree_t);

    ecs_os_free(ptree->first_65k);
    ecs_os_free(ptree);
    return result;
}

ecs_ptree_iter_t ecs_ptiny_iter(
    ecs_ptree_t *ptree)
{
    return (ecs_ptree_iter_t){
        .ptree = ptree,
        .frames[0] = &ptree->root
    };
}

ecs_ptree_iter_t ecs_ptree_iter(
    ecs_ptree_t *ptree)
{
    return (ecs_ptree_iter_t){
        .ptree = ptree,
        .frames[0] = &ptree->root,
        .index = (uint64_t)ptree->min_65k - 1
    };
}

void* _ecs_ptiny_next(
    ecs_ptree_iter_t *it,
    ecs_size_t elem_size)
{
    int8_t sp = it->sp;
    uint16_t cur_page = it->cur_page[sp];
    int32_t cur_elem = it->cur_elem;
    
    do {
        page_t *frame = it->frames[sp];

        if (cur_page < frame->pages.length) {
            page_t *page = ECS_OFFSET(frame->pages.data, 
                cur_page * sizeof(page_t));

            if (cur_elem < page->data.length) {
                void *result = ECS_OFFSET(page->data.data, cur_elem * elem_size);
                ecs_assert(result != NULL, ECS_INTERNAL_ERROR);
                it->index = index_from_it(it, page);
                it->cur_elem ++;
                return result;
            }

            if (page->pages.length) {
                sp = ++ it->sp;
                ecs_assert(sp < 3, ECS_INTERNAL_ERROR);
                cur_page = it->cur_page[sp] = 0;
                it->frames[sp] = page;
                cur_elem = it->cur_elem = 0;
                continue;
            }

            if (cur_page < (frame->pages.length - 1)) {
                cur_page = ++ it->cur_page[sp];
                cur_elem = it->cur_elem = 0;
                continue;
            }
        }

        if (sp) {
            sp = -- it->sp;
            cur_page = ++ it->cur_page[sp];
            continue;
        }

        return NULL;
    } while (1);
}

void* _ecs_ptree_next(
    ecs_ptree_iter_t *it,
    ecs_size_t elem_size)
{
    ecs_ptree_t *pt = it->ptree;
    if ((it->index == (uint64_t)-1) || (it->index < pt->max_65k)) {
        uint64_t index = ++ it->index;
        return ECS_OFFSET(pt->first_65k, index * pt->elem_size);
    } else {
        return _ecs_ptiny_next(it, elem_size);
    }
}

void* _ecs_ptiny_ensure(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index)
{
    ecs_assert(elem_size == (ecs_size_t)ptree->elem_size, ECS_INVALID_PARAMETER);

    addr_t addr = to_addr(index);
    page_t *p = get_or_create_page(&ptree->root, addr.value, page_count(index));
    ecs_assert(p != NULL, ECS_INTERNAL_ERROR);
    void *data = array_ensure(&p->data, elem_size, addr.value[0]);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);
    return data;
}

void* _ecs_ptree_ensure(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index)
{
    ecs_assert(elem_size == (ecs_size_t)ptree->elem_size, ECS_INVALID_PARAMETER);
    ecs_assert(ptree->first_65k != NULL, ECS_INVALID_PARAMETER);

    if (index < 65536) {
        if (index > ptree->max_65k) {
            ptree->max_65k = (uint16_t)index;
        }
        if (index < ptree->min_65k) {
            ptree->min_65k = (uint16_t)index;
        }
        return ECS_OFFSET(ptree->first_65k, (uint8_t)elem_size * index);
    } else {
       return _ecs_ptiny_ensure(ptree, (uint8_t)elem_size, index);
    }
}

void* _ecs_ptiny_get(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index)
{
    ecs_assert(elem_size == (ecs_size_t)ptree->elem_size, ECS_INVALID_PARAMETER);

    addr_t addr = to_addr(index);
    page_t *p = get_page(&ptree->root, addr.value, page_count(index));
    if (!p) {
        return NULL;
    }
    return array_get(&p->data, elem_size, addr.value[0]);
}

void* _ecs_ptree_get(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index)
{
    ecs_assert(elem_size == (ecs_size_t)ptree->elem_size, ECS_INVALID_PARAMETER);
    ecs_assert(ptree->first_65k != NULL, ECS_INTERNAL_ERROR);

    if (index < 65536) {
        return ECS_OFFSET(ptree->first_65k, (uint8_t)elem_size * index);
    } else {
        return _ecs_ptiny_get(ptree, (uint8_t)elem_size, index);
    }
}


/* -- Private functions -- */

/* O(n) algorithm to check whether type 1 is equal or superset of type 2 */
ecs_entity_t ecs_type_contains(
    ecs_world_t *world,
    ecs_type_t type_1,
    ecs_type_t type_2,
    bool match_all)
{
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);
    ecs_get_stage(&world);
    
    if (!type_1) {
        return 0;
    }

    ecs_assert(type_2 != NULL, ECS_INTERNAL_ERROR);

    if (type_1 == type_2) {
        return *(ecs_vector_first(type_1, ecs_entity_t));
    }

    int32_t i_2, i_1 = 0;
    ecs_entity_t e1 = 0;
    ecs_entity_t *t1_array = ecs_vector_first(type_1, ecs_entity_t);
    ecs_entity_t *t2_array = ecs_vector_first(type_2, ecs_entity_t);

    ecs_assert(t1_array != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(t2_array != NULL, ECS_INTERNAL_ERROR);

    int32_t t1_count = ecs_vector_count(type_1);
    int32_t t2_count = ecs_vector_count(type_2);

    for (i_2 = 0; i_2 < t2_count; i_2 ++) {
        ecs_entity_t e2 = t2_array[i_2];

        if (i_1 >= t1_count) {
            return 0;
        }

        e1 = t1_array[i_1];

        if (e2 > e1) {
            do {
                i_1 ++;
                if (i_1 >= t1_count) {
                    return 0;
                }
                e1 = t1_array[i_1];
            } while (e2 > e1);
        }

        if (e1 != e2) {
            if (e1 != e2) {
                if (match_all) return 0;
            } else if (!match_all) {
                return e1;
            }
        } else {
            if (!match_all) return e1;
            i_1 ++;
            if (i_1 < t1_count) {
                e1 = t1_array[i_1];
            }
        }
    }

    if (match_all) {
        return e1;
    } else {
        return 0;
    }
}

/* -- Public API -- */

int32_t ecs_type_index_of(
    ecs_type_t type,
    ecs_entity_t entity)
{
    ecs_vector_each(type, ecs_entity_t, c_ptr, {
        if (*c_ptr == entity) {
            return c_ptr_i; 
        }
    });

    return -1;
}

static
int match_entity(
    const ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t e,
    ecs_entity_t match_with)
{
    (void)world;
    ecs_entity_t role = ecs_get_role(match_with);

    if (role) {
        if (!ecs_has_role(role, e)) {
            return 0;
        }     

        ecs_entity_t id = match_with & ECS_ENTITY_MASK;
        ecs_entity_t comp = e & ECS_ENTITY_MASK;

        if (id == EcsWildcard) {
            ecs_entity_t *ids = ecs_vector_first(type, ecs_entity_t);
            int32_t i, count = ecs_vector_count(type);

            for (i = 0; i < count; i ++) {
                if (comp == ids[i]) {
                    return 2;
                }
            }

            return -1;
        }
    }

    if (e == match_with) {
        return 1;
    }

    return 0;
}

static
bool search_type(
    const ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t entity,
    bool owned)
{
    (void)owned;
    
    if (!type) {
        return false;
    }

    if (!entity) {
        return true;
    }

    ecs_entity_t *ids = ecs_vector_first(type, ecs_entity_t);
    int32_t i, count = ecs_vector_count(type);
    int matched = 0;

    for (i = 0; i < count; i ++) {
        int ret = match_entity(world, type, ids[i], entity);
        switch(ret) {
        case 0: break;
        case 1: return true;
        case -1: return false;
        case 2: matched ++; break;
        default: ecs_abort(ECS_INTERNAL_ERROR);
        }
    }

    return matched != 0;
}

bool ecs_type_has_entity(
    const ecs_world_t *world,
    ecs_type_t type,
    ecs_entity_t entity)
{
    return search_type(world, type, entity, false);
}

char* ecs_type_str(
    ecs_world_t *world,
    ecs_type_t type)
{
    if (!type) {
        return ecs_os_strdup("");
    }

    ecs_vector_t *chbuf = ecs_vector_new(char, 32);
    char *dst;

    ecs_entity_t *entities = ecs_vector_first(type, ecs_entity_t);
    int32_t i, count = ecs_vector_count(type);

    for (i = 0; i < count; i ++) {
        ecs_entity_t e = entities[i];
        char buffer[256];
        ecs_size_t len;

        if (i) {
            *(char*)ecs_vector_add(&chbuf, char) = ',';
        }

        if (e == 1) {
            ecs_os_strcpy(buffer, "EcsComponent");
            len = ecs_os_strlen("EcsComponent");
        } else {
            len = ecs_from_size_t(ecs_entity_str(world, e, buffer, 256));
        }

        dst = ecs_vector_addn(&chbuf, char, len);
        ecs_os_memcpy(dst, buffer, len);
    }

    *(char*)ecs_vector_add(&chbuf, char) = '\0';

    char* result = ecs_os_strdup(ecs_vector_first(chbuf, char));
    ecs_vector_free(chbuf);
    return result;
}

ecs_entities_t ecs_type_to_entities(
    ecs_type_t type)
{
    return (ecs_entities_t){
        .array = ecs_vector_first(type, ecs_entity_t),
        .count = ecs_vector_count(type)
    };
}

void ecs_os_api_impl(ecs_os_api_t *api);

static bool ecs_os_api_initialized = false;
static int ecs_os_api_init_count = 0;

ecs_os_api_t ecs_os_api;

int64_t ecs_os_api_malloc_count = 0;
int64_t ecs_os_api_realloc_count = 0;
int64_t ecs_os_api_calloc_count = 0;
int64_t ecs_os_api_free_count = 0;

void ecs_os_set_api(
    ecs_os_api_t *os_api)
{
    if (!ecs_os_api_initialized) {
        ecs_os_api = *os_api;
        ecs_os_api_initialized = true;
    }
}

void ecs_os_init(void)
{
    if (!ecs_os_api_initialized) {
        ecs_os_set_api_defaults();
    }
    
    if (!(ecs_os_api_init_count ++)) {
        if (ecs_os_api.init_) {
            ecs_os_api.init_();
        }
    }
}

void ecs_os_fini(void) {
    if (!--ecs_os_api_init_count) {
        if (ecs_os_api.fini_) {
            ecs_os_api.fini_();
        }
    }
}

static
void ecs_log(const char *fmt, va_list args) {
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
}

static
void ecs_log_error(const char *fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

static
void ecs_log_debug(const char *fmt, va_list args) {
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
}

static
void ecs_log_warning(const char *fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

void ecs_os_dbg(const char *fmt, ...) {
#ifndef NDEBUG
    va_list args;
    va_start(args, fmt);
    if (ecs_os_api.log_debug_) {
        ecs_os_api.log_debug_(fmt, args);
    }
    va_end(args);
#else
    (void)fmt;
#endif
}

void ecs_os_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (ecs_os_api.log_warning_) {
        ecs_os_api.log_warning_(fmt, args);
    }
    va_end(args);
}

void ecs_os_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (ecs_os_api.log_) {
        ecs_os_api.log_(fmt, args);
    }
    va_end(args);
}

void ecs_os_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (ecs_os_api.log_error_) {
        ecs_os_api.log_error_(fmt, args);
    }
    va_end(args);
}

void ecs_os_gettime(ecs_time_t *time)
{
    uint64_t now = ecs_os_time_now();
    uint64_t sec = now / 1000000000;

    assert(sec < UINT32_MAX);
    assert((now - sec * 1000000000) < UINT32_MAX);

    time->sec = (uint32_t)sec;
    time->nanosec = (uint32_t)(now - sec * 1000000000);
}

static
void* ecs_os_api_malloc(ecs_size_t size) {
    ecs_os_api_malloc_count ++;
    ecs_assert(size > 0, ECS_INVALID_PARAMETER);
    return malloc((size_t)size);
}

static
void* ecs_os_api_calloc(ecs_size_t size) {
    ecs_os_api_calloc_count ++;
    ecs_assert(size > 0, ECS_INVALID_PARAMETER);
    return calloc(1, (size_t)size);
}

static
void* ecs_os_api_realloc(void *ptr, ecs_size_t size) {
    ecs_assert(size > 0, ECS_INVALID_PARAMETER);

    if (ptr) {
        ecs_os_api_realloc_count ++;
    } else {
        /* If not actually reallocing, treat as malloc */
        ecs_os_api_malloc_count ++; 
    }
    
    return realloc(ptr, (size_t)size);
}

static
void ecs_os_api_free(void *ptr) {
    if (ptr) {
        ecs_os_api_free_count ++;
    }
    free(ptr);
}

static
char* ecs_os_api_strdup(const char *str) {
    int len = ecs_os_strlen(str);
    char *result = ecs_os_malloc(len + 1);
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);
    ecs_os_strcpy(result, str);
    return result;
}

static
char* ecs_os_api_module_to_dl(const char *module) {
    (void)module;
    return NULL;
}

static
char* ecs_os_api_module_to_etc(const char *module) {
    (void)module;
    return NULL;
}

void ecs_os_set_api_defaults(void)
{
    /* Don't overwrite if already initialized */
    if (ecs_os_api_initialized != 0) {
        return;
    }

    ecs_os_time_setup();
    
    /* Memory management */
    ecs_os_api.malloc_ = ecs_os_api_malloc;
    ecs_os_api.free_ = ecs_os_api_free;
    ecs_os_api.realloc_ = ecs_os_api_realloc;
    ecs_os_api.calloc_ = ecs_os_api_calloc;

    /* Strings */
    ecs_os_api.strdup_ = ecs_os_api_strdup;

    /* Time */
    ecs_os_api.sleep_ = ecs_os_time_sleep;
    ecs_os_api.get_time_ = ecs_os_gettime;

    /* Logging */
    ecs_os_api.log_ = ecs_log;
    ecs_os_api.log_error_ = ecs_log_error;
    ecs_os_api.log_debug_ = ecs_log_debug;
    ecs_os_api.log_warning_ = ecs_log_warning;

    /* Modules */
    if (!ecs_os_api.module_to_dl_) {
        ecs_os_api.module_to_dl_ = ecs_os_api_module_to_dl;
    }

    if (!ecs_os_api.module_to_etc_) {
        ecs_os_api.module_to_etc_ = ecs_os_api_module_to_etc;
    }

    ecs_os_api.abort_ = abort;
}

bool ecs_os_has_heap(void) {
    return 
        (ecs_os_api.malloc_ != NULL) &&
        (ecs_os_api.calloc_ != NULL) &&
        (ecs_os_api.realloc_ != NULL) &&
        (ecs_os_api.free_ != NULL);
}

bool ecs_os_has_threading(void) {
    return
        (ecs_os_api.mutex_new_ != NULL) &&
        (ecs_os_api.mutex_free_ != NULL) &&
        (ecs_os_api.mutex_lock_ != NULL) &&
        (ecs_os_api.mutex_unlock_ != NULL) &&
        (ecs_os_api.cond_new_ != NULL) &&
        (ecs_os_api.cond_free_ != NULL) &&
        (ecs_os_api.cond_wait_ != NULL) &&
        (ecs_os_api.cond_signal_ != NULL) &&
        (ecs_os_api.cond_broadcast_ != NULL) &&
        (ecs_os_api.thread_new_ != NULL) &&
        (ecs_os_api.thread_join_ != NULL);   
}

bool ecs_os_has_time(void) {
    return 
        (ecs_os_api.get_time_ != NULL) &&
        (ecs_os_api.sleep_ != NULL);
}

bool ecs_os_has_logging(void) {
    return 
        (ecs_os_api.log_ != NULL) &&
        (ecs_os_api.log_error_ != NULL) &&
        (ecs_os_api.log_debug_ != NULL) &&
        (ecs_os_api.log_warning_ != NULL);
}

bool ecs_os_has_dl(void) {
    return 
        (ecs_os_api.dlopen_ != NULL) &&
        (ecs_os_api.dlproc_ != NULL) &&
        (ecs_os_api.dlclose_ != NULL);  
}

bool ecs_os_has_modules(void) {
    return 
        (ecs_os_api.module_to_dl_ != NULL) &&
        (ecs_os_api.module_to_etc_ != NULL);
}

ecs_entity_t ecs_component_id_from_id(
    ecs_world_t *world,
    ecs_entity_t e)
{
    ecs_entity_t role = ecs_get_role(e);
    if (role) {
        if (ecs_has(world, role, EcsComponent)) {
            return role;
        }

        e &= ECS_ENTITY_MASK;
    }

    if (ecs_has(world, e, EcsComponent)) {
        return e;
    }

    return 0;
}

const EcsComponent* ecs_component_from_id(
    ecs_world_t *world,
    ecs_entity_t e)
{
    ecs_entity_t role = ecs_get_role(e);

    const EcsComponent *component = NULL;
    if (role) {
        component = ecs_get(world, role, EcsComponent);
        e &= ECS_ENTITY_MASK;
    }

    if (!component) {
        component = ecs_get(world, e, EcsComponent);
    }

    return component;
}

/* Count number of columns with data (excluding tags) */
static
int32_t data_column_count(
    ecs_world_t * world,
    ecs_table_t * table)
{
    int32_t count = 0;
    ecs_vector_each(table->type, ecs_entity_t, c_ptr, {
        ecs_entity_t component = *c_ptr;

        /* Typically all components will be clustered together at the start of
         * the type as components are created from a separate id pool, and type
         * vectors are sorted. 
         * Explicitly check for EcsComponent and EcsName since the ecs_has check
         * doesn't work during bootstrap. */
        const EcsComponent *c = NULL;
        if ((component == ecs_typeid(EcsComponent)) || 
            (component == ecs_typeid(EcsName)) || 
            (c = ecs_component_from_id(world, component)) != NULL) 
        {
            if (!c || c->size) {
                count = c_ptr_i + 1;
            }
        }
    });

    return count;
}

static
ecs_type_t entities_to_type(
    ecs_entities_t *entities)
{
    if (entities->count) {
        ecs_vector_t *result = NULL;
        ecs_vector_set_count(&result, ecs_entity_t, entities->count);
        ecs_entity_t *array = ecs_vector_first(result, ecs_entity_t);
        ecs_os_memcpy(array, entities->array, ECS_SIZEOF(ecs_entity_t) * entities->count);
        return result;
    } else {
        return NULL;
    }
}

static
void init_edges(
    ecs_world_t * world,
    ecs_table_t * table)
{
    (void)world;

    ecs_entity_t *entities = ecs_vector_first(table->type, ecs_entity_t);
    int32_t count = ecs_vector_count(table->type);

    table->edges = ecs_ptiny_new(ecs_edge_t);
    
    int32_t i;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = entities[i];

        ecs_edge_t *edge = ecs_ptiny_ensure(table->edges, ecs_edge_t, e);
        edge->add = table;

        /* As we're iterating over the table components, also set the table
         * flags. These allow us to quickly determine if the table contains
         * data that needs to be handled in a special way, like prefabs or 
         * containers */
        if (e <= EcsLastInternalComponentId) {
            table->flags |= EcsTableHasBuiltins;
        }

        if (e == EcsDisabled) {
            table->flags |= EcsTableIsDisabled;
        }

        if (e == ecs_typeid(EcsComponent)) {
            table->flags |= EcsTableHasComponentData;
        }
    }
}

static
void init_table(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_entities_t * entities)
{
    (void)world;

    table->type = entities_to_type(entities);
    table->column_count = data_column_count(world, table);

    uint64_t hash = 0;
    ecs_hash(entities->array, entities->count * ECS_SIZEOF(ecs_entity_t), &hash);
    ecs_map_set(world->store.table_map, hash, &table);

    init_edges(world, table);
}

static
ecs_table_t *create_table(
    ecs_world_t * world,
    ecs_entities_t * entities)
{
    ecs_table_t *result = ecs_sparse_add(world->store.tables, ecs_table_t);
    result->id = ecs_to_u32(ecs_sparse_last_id(world->store.tables));

    ecs_assert(result != NULL, ECS_INTERNAL_ERROR);
    init_table(world, result, entities);

    return result;
}

static
void remove_entity_from_type(
    ecs_type_t type,
    ecs_entity_t remove,
    ecs_entities_t *out)
{
    int32_t count = ecs_vector_count(type);
    ecs_entity_t *array = ecs_vector_first(type, ecs_entity_t);

    int32_t i, el = 0;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = array[i];
        if (e != remove) {
            out->array[el ++] = e;
            ecs_assert(el <= count, ECS_INTERNAL_ERROR);
        }
    }

    out->count = el;
}

static
void add_entity_to_type(
    ecs_type_t type,
    ecs_entity_t add,
    ecs_entities_t *out)
{
    int32_t count = ecs_vector_count(type);
    ecs_entity_t *array = ecs_vector_first(type, ecs_entity_t);    
    bool added = false;

    int32_t i, el = 0;
    for (i = 0; i < count; i ++) {
        ecs_entity_t e = array[i];

        if (e > add && !added) {
            out->array[el ++] = add;
            added = true;
        }
        
        out->array[el ++] = e;
        ecs_assert(el <= out->count, ECS_INTERNAL_ERROR);
    }

    if (!added) {
        out->array[el ++] = add;
    }

    out->count = el;
    ecs_assert(out->count != 0, ECS_INTERNAL_ERROR);
}

static
ecs_table_t *find_or_create_remove(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t remove)
{
    ecs_type_t type = node->type;
    int32_t count = ecs_vector_count(type);

    ecs_entities_t entities = {
        .array = ecs_os_alloca(ECS_SIZEOF(ecs_entity_t) * count),
        .count = count
    };

    remove_entity_from_type(type, remove, &entities);

    ecs_table_t *result = ecs_table_find_or_create(world, &entities);
    if (!result) {
        return NULL;
    }

    if (result != node) {
        ecs_edge_t *edge = ecs_ptiny_ensure(result->edges, ecs_edge_t, remove);
        if (!edge->add) {
            edge->add = node;
        }
    }

    return result;    
}

static
ecs_table_t *find_or_create_add(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t add)
{
    ecs_type_t type = node->type;
    int32_t count = ecs_vector_count(type);

    ecs_entities_t entities = {
        .array = ecs_os_alloca(ECS_SIZEOF(ecs_entity_t) * (count + 1)),
        .count = count + 1
    };

    add_entity_to_type(type, add, &entities);

    ecs_table_t *result = ecs_table_find_or_create(world, &entities);
    
    if (result != node) {
        ecs_edge_t *edge = ecs_ptiny_ensure(result->edges, ecs_edge_t, add);
        if (!edge->remove) {
            edge->remove = node;
        }
    }

    return result;
}

ecs_table_t* ecs_table_traverse_remove(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t e)    
{
    node = node ? node : &world->store.root;

    ecs_edge_t *edge = ecs_ptiny_get(node->edges, ecs_edge_t, e);
    if (!edge) {
        edge = ecs_ptiny_ensure(node->edges, ecs_edge_t, e);
    }

    ecs_table_t *next = edge->remove;

    if (!next) {
        next = find_or_create_remove(world, node, e);
        ecs_assert(next != NULL, ECS_INTERNAL_ERROR);
        edge->add = next;
    }

    return next;
}

ecs_table_t* ecs_table_traverse_add(
    ecs_world_t * world,
    ecs_table_t * node,
    ecs_entity_t e)    
{
    node = node ? node : &world->store.root;

    ecs_edge_t *edge = ecs_ptiny_get(node->edges, ecs_edge_t, e);
    if (!edge) {
        edge = ecs_ptiny_ensure(node->edges, ecs_edge_t, e);
    }
    ecs_table_t *next = edge->add;

    if (!next) {
        next = find_or_create_add(world, node, e);
        ecs_assert(next != NULL, ECS_INTERNAL_ERROR);
        edge->add = next;
    }

    return next;
}

static
int ecs_entity_compare(
    const void *e1,
    const void *e2)
{
    ecs_entity_t v1 = *(ecs_entity_t*)e1;
    ecs_entity_t v2 = *(ecs_entity_t*)e2;
    if (v1 < v2) {
        return -1;
    } else if (v1 > v2) {
        return 1;
    } else {
        return 0;
    }
}

static
bool ecs_entity_array_is_ordered(
    ecs_entities_t *entities)
{
    ecs_entity_t prev = 0;
    ecs_entity_t *array = entities->array;
    int32_t i, count = entities->count;

    for (i = 0; i < count; i ++) {
        if (!array[i] && !prev) {
            continue;
        }
        if (array[i] <= prev) {
            return false;
        }
        prev = array[i];
    }    

    return true;
}

static
int32_t ecs_entity_array_dedup(
    ecs_entity_t *array,
    int32_t count)
{
    int32_t j, k;
    ecs_entity_t prev = array[0];

    for (k = j = 1; k < count; j ++, k++) {
        ecs_entity_t e = array[k];
        if (e == prev) {
            k ++;
        }

        array[j] = e;
        prev = e;
    }

    return count - (k - j);
}

static
ecs_table_t *find_or_create(
    ecs_world_t * world,
    ecs_entities_t * entities)
{    
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);

    /* Make sure array is ordered and does not contain duplicates */
    int32_t type_count = entities->count;
    ecs_entity_t *ordered = NULL;

    if (!type_count) {
        return &world->store.root;
    }

    if (!ecs_entity_array_is_ordered(entities)) {
        ecs_size_t size = ECS_SIZEOF(ecs_entity_t) * type_count;
        ordered = ecs_os_alloca(size);
        ecs_os_memcpy(ordered, entities->array, size);
        qsort(ordered, (size_t)type_count, sizeof(ecs_entity_t), ecs_entity_compare);
        type_count = ecs_entity_array_dedup(ordered, type_count);
    } else {
        ordered = entities->array;
    }

    uint64_t hash = 0;
    ecs_hash(entities->array, entities->count * ECS_SIZEOF(ecs_entity_t), &hash);
    ecs_table_t *table = ecs_map_get_ptr(world->store.table_map, ecs_table_t*, hash);
    if (table) {
        return table;
    }

    ecs_entities_t ordered_entities = {
        .array = ordered,
        .count = type_count
    };

    /* If we get here, the table has not been found. It has to be created. */
    
    ecs_table_t *result = create_table(world, &ordered_entities);

    ecs_assert(ordered_entities.count == ecs_vector_count(result->type), 
        ECS_INTERNAL_ERROR);

    return result;
}

ecs_table_t* ecs_table_find_or_create(
    ecs_world_t * world,
    ecs_entities_t * components)
{
    ecs_assert(world != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(!world->in_progress, ECS_INTERNAL_ERROR);

    return find_or_create(world, components);
}

ecs_table_t* ecs_table_from_type(
    ecs_world_t *world,
    ecs_type_t type)
{
    ecs_entities_t components = ecs_type_to_entities(type);
    return ecs_table_find_or_create(
        world, &components);
}

void ecs_init_root_table(
    ecs_world_t *world,
    ecs_table_t *root)
{
    ecs_entities_t entities = {
        .array = NULL,
        .count = 0
    };

    init_table(world, root, &entities);
}

void ecs_table_clear_edges(
    ecs_table_t *table)
{
    (void)table;
}

#define LOAD_FACTOR (1.5)
#define KEY_SIZE (ECS_SIZEOF(ecs_map_key_t))
#define BUCKET_COUNT (8)
#define ELEM_SIZE(elem_size) (KEY_SIZE + elem_size)
#define BUCKET_SIZE(elem_size, offset)\
    (offset + BUCKET_COUNT * (ELEM_SIZE(elem_size)))

#define NEXT_ELEM(elem, elem_size) \
    ECS_OFFSET(elem, ELEM_SIZE(elem_size))

#define GET_ELEM(array, elem_size, index) \
    ECS_OFFSET(array, ELEM_SIZE(elem_size) * index)

#define PAYLOAD_ARRAY(bucket, offset) \
    ECS_OFFSET(bucket, offset)

#define PAYLOAD(elem) \
    ECS_OFFSET(elem, KEY_SIZE)

struct ecs_bucket_t {
    int32_t count;
};

struct ecs_map_t {
    ecs_sparse_t *buckets;
    int32_t elem_size;
    int32_t type_elem_size;
    int32_t bucket_size;
    int32_t bucket_count;
    int32_t count;
    int32_t offset;
};

static
int32_t get_bucket_count(
    int32_t element_count)
{
    return ecs_next_pow_of_2((int32_t)((float)element_count * LOAD_FACTOR));
}

static
uint64_t get_bucket_id(
    int32_t bucket_count,
    ecs_map_key_t key) 
{
    ecs_assert(bucket_count > 0, ECS_INTERNAL_ERROR);
    uint64_t result = key & ((uint64_t)bucket_count - 1);
    ecs_assert(result < INT32_MAX, ECS_INTERNAL_ERROR);
    return result;
}

static
ecs_bucket_t* find_bucket(
    const ecs_map_t *map,
    ecs_map_key_t key)
{
    ecs_sparse_t *buckets = map->buckets;
    int32_t bucket_count = map->bucket_count;
    if (!bucket_count) {
        return NULL;
    }

    uint64_t bucket_id = get_bucket_id(bucket_count, key);

    return _ecs_sparse_get(buckets, 0, bucket_id);
}

static
ecs_bucket_t* find_or_create_bucket(
    ecs_map_t *map,
    ecs_map_key_t key)
{
    ecs_sparse_t *buckets = map->buckets;
    int32_t bucket_count = map->bucket_count;

    if (!bucket_count) {
        ecs_sparse_set_size(buckets, 8);
        bucket_count = 8;
    }

    uint64_t bucket_id = get_bucket_id(bucket_count, key);
    return _ecs_sparse_get_or_create(buckets, 0, bucket_id);    
}

static
void remove_bucket(
    ecs_map_t *map,
    ecs_map_key_t key)
{
    int32_t bucket_count = map->bucket_count;
    uint64_t bucket_id = get_bucket_id(bucket_count, key);
    ecs_sparse_remove(map->buckets, bucket_id);
    ecs_sparse_set_generation(map->buckets, bucket_id);
}

static
int32_t add_to_bucket(
    ecs_bucket_t *bucket,
    ecs_size_t elem_size,
    int32_t offset,
    ecs_map_key_t key,
    void *payload)
{
    ecs_assert(bucket->count < BUCKET_COUNT, ECS_INTERNAL_ERROR);

    void *array = PAYLOAD_ARRAY(bucket, offset);
    ecs_map_key_t *elem = GET_ELEM(array, elem_size, bucket->count);
    *elem = key;
    ecs_os_memcpy(PAYLOAD(elem), payload, elem_size);
    return ++ bucket->count;
}

static
void remove_from_bucket(
    ecs_bucket_t *bucket,
    ecs_map_key_t key,
    ecs_size_t elem_size,
    int32_t offset,
    int32_t index)
{
    (void)key;

    ecs_assert(bucket->count != 0, ECS_INTERNAL_ERROR);
    ecs_assert(index < bucket->count, ECS_INTERNAL_ERROR);

    bucket->count--;

    if (index != bucket->count) {
        void *array = PAYLOAD_ARRAY(bucket, offset);
        ecs_map_key_t *elem = GET_ELEM(array, elem_size, index);
        ecs_map_key_t *last_elem = GET_ELEM(array, elem_size, bucket->count);

        ecs_assert(key == *elem, ECS_INTERNAL_ERROR);
        ecs_os_memcpy(elem, last_elem, ELEM_SIZE(elem_size));
    }
}

static
void rehash(
    ecs_map_t *map,
    int32_t bucket_count)
{
    bool rehash_again;

    ecs_assert(bucket_count != 0, ECS_INTERNAL_ERROR);
    ecs_assert(bucket_count > map->bucket_count, ECS_INTERNAL_ERROR);

    do {
        rehash_again = false;

        ecs_sparse_t *buckets = map->buckets;
        ecs_size_t elem_size = map->elem_size;
        int32_t offset = map->offset;

        ecs_sparse_set_size(buckets, bucket_count);
        map->bucket_count = bucket_count;

        /* Only iterate over old buckets with elements */
        int32_t b, filled_bucket_count = ecs_sparse_count(buckets);
        const uint64_t *indices = ecs_sparse_ids(buckets);

        /* Iterate backwards as elements could otherwise be moved to existing
         * buckets which could temporarily cause the number of elements in a
         * bucket to exceed BUCKET_COUNT. */
        for (b = filled_bucket_count - 1; b >= 0; b --) {
            uint64_t bucket_id = indices[b];
            ecs_bucket_t *bucket = _ecs_sparse_get(buckets, 0, bucket_id);
            ecs_assert(bucket != NULL, ECS_INTERNAL_ERROR);

            int i, count = bucket->count;
            ecs_map_key_t *array = PAYLOAD_ARRAY(bucket, offset);

            for (i = 0; i < count; i ++) {
                ecs_map_key_t *elem = GET_ELEM(array, elem_size, i);
                ecs_map_key_t key = *elem;
                uint64_t new_bucket_id = get_bucket_id(bucket_count, key);

                if (new_bucket_id != bucket_id) {
                    ecs_bucket_t *new_bucket = _ecs_sparse_get_or_create(
                        buckets, 0, new_bucket_id);
                    ecs_assert(new_bucket != NULL, ECS_INTERNAL_ERROR);

                    indices = ecs_sparse_ids(buckets);
                    ecs_assert(indices != NULL, ECS_INTERNAL_ERROR);

                    if (add_to_bucket(new_bucket, elem_size, offset, 
                        key, PAYLOAD(elem)) == BUCKET_COUNT) 
                    {
                        rehash_again = true;
                    }

                    remove_from_bucket(bucket, key, elem_size, offset, i);

                    count --;
                    i --;
                }
            }
        }

        bucket_count *= 2;        
    } while (rehash_again);
}

ecs_map_t* _ecs_map_new(
    ecs_size_t elem_size,
    ecs_size_t alignment, 
    int32_t element_count)
{
    ecs_map_t *result = ecs_os_calloc(ECS_SIZEOF(ecs_map_t) * 1);
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY);

    int32_t bucket_count = get_bucket_count(element_count);

    result->count = 0;
    result->type_elem_size = elem_size;

    if (elem_size < ECS_SIZEOF(ecs_map_key_t)) {
        result->elem_size = ECS_SIZEOF(ecs_map_key_t);
    } else {
        result->elem_size = elem_size;
    }
    
    if (alignment < ECS_SIZEOF(ecs_map_key_t)) {
        result->offset = ECS_SIZEOF(ecs_map_key_t);
    } else {
        result->offset = alignment;
    }

    result->bucket_count = bucket_count;
    result->buckets = _ecs_sparse_new(BUCKET_SIZE(elem_size, result->offset));

    return result;
}

void ecs_map_free(
    ecs_map_t *map)
{
    if (map) {
        ecs_sparse_free(map->buckets);
        ecs_os_free(map);
    }
}

void* _ecs_map_get(
    const ecs_map_t *map,
    ecs_size_t elem_size,
    ecs_map_key_t key)
{
    (void)elem_size;

    if (!map) {
        return NULL;
    }

    ecs_assert(elem_size == map->type_elem_size, ECS_INVALID_PARAMETER);

    ecs_bucket_t * bucket = find_bucket(map, key);

    if (!bucket) {
        return NULL;
    }

    ecs_map_key_t *elem = PAYLOAD_ARRAY(bucket, map->offset);

    uint8_t i = 0;
    while (i++ < bucket->count) {
        if (*elem == key) {
            return PAYLOAD(elem);
        }

        elem = NEXT_ELEM(elem, map->elem_size);
    }

    return NULL;
}

void* _ecs_map_get_ptr(
    const ecs_map_t *map,
    ecs_map_key_t key)
{
    void * ptr_ptr = _ecs_map_get(map, ECS_SIZEOF(void*), key);

    if (ptr_ptr) {
        return *(void**)ptr_ptr;
    } else {
        return NULL;
    }
}

void _ecs_map_set(
    ecs_map_t *map,
    ecs_size_t elem_size,
    ecs_map_key_t key,
    const void *payload)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(elem_size == map->type_elem_size, ECS_INVALID_PARAMETER);

    ecs_bucket_t * bucket = find_or_create_bucket(map, key);
    ecs_assert(bucket != NULL, ECS_INTERNAL_ERROR);
    
    int32_t bucket_count = bucket->count;
    void *array = PAYLOAD_ARRAY(bucket, map->offset);
    ecs_map_key_t *elem = array, *found = NULL;

    uint8_t i = 0;
    while (i++ < bucket_count) {
        if (*elem == key) {
            found = elem;
            break;
        }

        elem = NEXT_ELEM(elem, map->elem_size);
    }

    if (!found) {
        if (bucket->count == BUCKET_COUNT) {
            /* Can't fit in current bucket, need to grow the map first */
            rehash(map, map->bucket_count * 2);
            _ecs_map_set(map, elem_size, key, payload);
        } else {
            ecs_assert(bucket->count < BUCKET_COUNT, ECS_INTERNAL_ERROR);

            bucket_count = ++bucket->count;
            int32_t map_count = ++map->count;
            
            *elem = key;
            ecs_os_memcpy(PAYLOAD(elem), payload, elem_size);

            int32_t target_bucket_count = get_bucket_count(map_count);
            int32_t map_bucket_count = map->bucket_count;

            if (bucket_count == BUCKET_COUNT) {
                rehash(map, map_bucket_count * 2);
            } else {
                if (target_bucket_count > map_bucket_count) {
                    rehash(map, target_bucket_count);
                }
            }            
        }
    } else {
        *found = key;
        ecs_os_memcpy(PAYLOAD(found), payload, elem_size);
    }

    ecs_assert(map->bucket_count != 0, ECS_INTERNAL_ERROR);
}

void ecs_map_remove(
    ecs_map_t *map,
    ecs_map_key_t key)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);

    ecs_bucket_t * bucket = find_bucket(map, key);
    if (!bucket) {
        return;
    }
   
    ecs_size_t elem_size = map->elem_size;
    void *array = PAYLOAD_ARRAY(bucket, map->offset);
    ecs_map_key_t *elem = array;
    int32_t bucket_count = bucket->count;

    if (!bucket_count) {
        return;
    }

    uint8_t i = 0;
    do {
        if (*elem == key) {
            ecs_map_key_t *last_elem = GET_ELEM(array, elem_size, (bucket_count - 1));
            if (last_elem > elem) {
                ecs_os_memcpy(elem, last_elem, ELEM_SIZE(elem_size));
            }

            map->count --;
            if (!--bucket->count) {
                remove_bucket(map, key);
            }

            break;
        }

        elem = NEXT_ELEM(elem, elem_size);
    } while (++i < bucket_count);   
}

int32_t ecs_map_count(
    const ecs_map_t *map)
{
    return map ? map->count : 0;
}

int32_t ecs_map_bucket_count(
    const ecs_map_t *map)
{
    return map ? map->bucket_count : 0;
}

void ecs_map_clear(
    ecs_map_t *map)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    ecs_sparse_clear(map->buckets);
    map->count = 0;
}

ecs_map_iter_t ecs_map_iter(
    const ecs_map_t *map)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);

    return (ecs_map_iter_t){
        .map = map,
        .bucket = NULL,
        .bucket_index = 0,
        .element_index = 0
    };
}

void* _ecs_map_next(
    ecs_map_iter_t *iter,
    ecs_size_t elem_size,
    ecs_map_key_t *key_out)
{
    const ecs_map_t *map = iter->map;
    
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    ecs_assert(!elem_size || elem_size == map->type_elem_size, ECS_INVALID_PARAMETER);
 
    ecs_bucket_t *bucket = iter->bucket;
    int32_t element_index = iter->element_index;
    elem_size = map->elem_size;

    do {
        if (!bucket) {
            int32_t bucket_index = iter->bucket_index;
            ecs_sparse_t *buckets = map->buckets;
            if (bucket_index < ecs_sparse_count(buckets)) {
                bucket = _ecs_sparse_get_elem(buckets, 0, bucket_index);
                iter->bucket = bucket;

                element_index = 0;
                iter->element_index = 0;
            } else {
                return NULL;
            }
        }

        if (element_index < bucket->count) {
            iter->element_index = element_index + 1;
            break;
        } else {
            bucket = NULL;
            iter->bucket_index ++;
        }
    } while (true);

    void *array = PAYLOAD_ARRAY(bucket, map->offset);
    ecs_map_key_t *elem = GET_ELEM(array, elem_size, element_index);
    
    if (key_out) {
        *key_out = *elem;
    }

    return PAYLOAD(elem);
}

void* _ecs_map_next_ptr(
    ecs_map_iter_t *iter,
    ecs_map_key_t *key_out)
{
    void *result = _ecs_map_next(iter, ECS_SIZEOF(void*), key_out);
    if (result) {
        return *(void**)result;
    } else {
        return NULL;
    }
}

void ecs_map_grow(
    ecs_map_t *map, 
    int32_t element_count)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    int32_t target_count = map->count + element_count;
    int32_t bucket_count = get_bucket_count(target_count);

    if (bucket_count > map->bucket_count) {
        rehash(map, bucket_count);
    }
}

void ecs_map_set_size(
    ecs_map_t *map, 
    int32_t element_count)
{    
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    int32_t bucket_count = get_bucket_count(element_count);

    if (bucket_count) {
        rehash(map, bucket_count);
    }
}

void ecs_map_memory(
    ecs_map_t *map, 
    int32_t *allocd,
    int32_t *used)
{
    ecs_assert(map != NULL, ECS_INVALID_PARAMETER);
    ecs_sparse_memory(map->buckets, allocd, used);

    if (allocd) {
        *allocd += ECS_SIZEOF(ecs_map_t);
    }

    if (used) {
        *used += ECS_SIZEOF(ecs_map_t);
    }
}

ecs_map_t* ecs_map_copy(
    const ecs_map_t *src)
{
    if (!src) {
        return NULL;
    }
    
    ecs_map_t *dst = ecs_os_memdup(src, ECS_SIZEOF(ecs_map_t));
    
    dst->buckets = ecs_sparse_copy(src->buckets);

    return dst;
}

int8_t ecs_to_i8(
    int64_t v)
{
    ecs_assert(v < INT8_MAX, ECS_INTERNAL_ERROR);
    return (int8_t)v;
}

int16_t ecs_to_i16(
    int64_t v)
{
    ecs_assert(v < INT16_MAX, ECS_INTERNAL_ERROR);
    return (int16_t)v;
}

uint32_t ecs_to_u32(
    uint64_t v)
{
    ecs_assert(v < UINT32_MAX, ECS_INTERNAL_ERROR);
    return (uint32_t)v;    
}

int32_t ecs_to_i32(
    uint64_t v)
{
    ecs_assert(v < INT32_MAX, ECS_INTERNAL_ERROR);
    return (int32_t)v;      
}

size_t ecs_to_size_t(
    int64_t size)
{
    ecs_assert(size >= 0, ECS_INTERNAL_ERROR);
    return (size_t)size;
}

ecs_size_t ecs_from_size_t(
    size_t size)
{
   ecs_assert(size < INT32_MAX, ECS_INTERNAL_ERROR); 
   return (ecs_size_t)size;
}

int32_t ecs_next_pow_of_2(
    int32_t n)
{
    n --;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n ++;

    return n;
}

/** Convert time to double */
double ecs_time_to_double(
    ecs_time_t t)
{
    double result;
    result = t.sec;
    return result + (double)t.nanosec / (double)1000000000;
}

ecs_time_t ecs_time_sub(
    ecs_time_t t1,
    ecs_time_t t2)
{
    ecs_time_t result;

    if (t1.nanosec >= t2.nanosec) {
        result.nanosec = t1.nanosec - t2.nanosec;
        result.sec = t1.sec - t2.sec;
    } else {
        result.nanosec = t1.nanosec - t2.nanosec + 1000000000;
        result.sec = t1.sec - t2.sec - 1;
    }

    return result;
}

void ecs_sleepf(
    double t)
{
    if (t > 0) {
        int sec = (int)t;
        int nsec = (int)((t - sec) * 1000000000);
        ecs_os_sleep(sec, nsec);
    }
}

double ecs_time_measure(
    ecs_time_t *start)
{
    ecs_time_t stop, temp;
    ecs_os_get_time(&stop);
    temp = stop;
    stop = ecs_time_sub(stop, *start);
    *start = temp;
    return ecs_time_to_double(stop);
}

void* ecs_os_memdup(
    const void *src, 
    ecs_size_t size) 
{
    if (!src) {
        return NULL;
    }
    
    void *dst = ecs_os_malloc(size);
    ecs_assert(dst != NULL, ECS_OUT_OF_MEMORY);
    ecs_os_memcpy(dst, src, size);  
    return dst;  
}

/*
    This code was taken from sokol_time.h 
    
    zlib/libpng license
    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution.
*/


static int ecs_os_time_initialized;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
static double _ecs_os_time_win_freq;
static LARGE_INTEGER _ecs_os_time_win_start;
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
static mach_timebase_info_data_t _ecs_os_time_osx_timebase;
static uint64_t _ecs_os_time_osx_start;
#else /* anything else, this will need more care for non-Linux platforms */
#include <time.h>
static uint64_t _ecs_os_time_posix_start;
#endif

/* prevent 64-bit overflow when computing relative timestamp
    see https://gist.github.com/jspohr/3dc4f00033d79ec5bdaf67bc46c813e3
*/
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
int64_t int64_muldiv(int64_t value, int64_t numer, int64_t denom) {
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}
#endif

void ecs_os_time_setup(void) {
    if ( ecs_os_time_initialized) {
        return;
    }
    
    ecs_os_time_initialized = 1;
    #if defined(_WIN32)
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&_ecs_os_time_win_start);
        _ecs_os_time_win_freq = (double)freq.QuadPart / 1000000000.0;
    #elif defined(__APPLE__) && defined(__MACH__)
        mach_timebase_info(&_ecs_os_time_osx_timebase);
        _ecs_os_time_osx_start = mach_absolute_time();
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        _ecs_os_time_posix_start = (uint64_t)ts.tv_sec*1000000000 + (uint64_t)ts.tv_nsec; 
    #endif
}

uint64_t ecs_os_time_now(void) {
    ecs_assert(ecs_os_time_initialized != 0, ECS_INTERNAL_ERROR);

    uint64_t now;

    #if defined(_WIN32)
        LARGE_INTEGER qpc_t;
        QueryPerformanceCounter(&qpc_t);
        now = (uint64_t)(qpc_t.QuadPart / _ecs_os_time_win_freq);
    #elif defined(__APPLE__) && defined(__MACH__)
        now = mach_absolute_time();
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now = ((uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec);
    #endif

    return now;
}

void ecs_os_time_sleep(
    int32_t sec, 
    int32_t nanosec) 
{
#ifndef _WIN32
    struct timespec sleepTime;
    ecs_assert(sec >= 0, ECS_INTERNAL_ERROR);
    ecs_assert(nanosec >= 0, ECS_INTERNAL_ERROR);

    sleepTime.tv_sec = sec;
    sleepTime.tv_nsec = nanosec;
    if (nanosleep(&sleepTime, NULL)) {
        ecs_os_err("nanosleep failed");
    }
#else
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -((int64_t)sec * 10000000 + (int64_t)nanosec / 100);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#endif
}


#if defined(_WIN32)

static ULONG win32_current_resolution;

void ecs_increase_timer_resolution(bool enable)
{
    HMODULE hntdll = GetModuleHandle((LPCTSTR)"ntdll.dll");
    if (!hntdll) {
        return;
    }

    LONG (__stdcall *pNtSetTimerResolution)(
        ULONG desired, BOOLEAN set, ULONG * current);

    pNtSetTimerResolution = (LONG(__stdcall*)(ULONG, BOOLEAN, ULONG*))
        GetProcAddress(hntdll, "NtSetTimerResolution");

    if(!pNtSetTimerResolution) {
        return;
    }

    ULONG current, resolution = 10000; /* 1 ms */

    if (!enable && win32_current_resolution) {
        pNtSetTimerResolution(win32_current_resolution, 0, &current);
        win32_current_resolution = 0;
        return;
    } else if (!enable) {
        return;
    }

    if (resolution == win32_current_resolution) {
        return;
    }

    if (win32_current_resolution) {
        pNtSetTimerResolution(win32_current_resolution, 0, &current);
    }

    if (pNtSetTimerResolution(resolution, 1, &current)) {
        /* Try setting a lower resolution */
        resolution *= 2;
        if(pNtSetTimerResolution(resolution, 1, &current)) return;
    }

    win32_current_resolution = resolution;
}

#else
void ecs_increase_timer_resolution(bool enable)
{
    (void)enable;
    return;
}
#endif

/* Component lifecycle actions for EcsName */
void EcsName_ctor(EcsName *ptr, size_t size, int32_t count) {
    memset(ptr, 0, size * ecs_to_size_t(count));
}

void EcsName_dtor(EcsName *ptr, size_t size, int32_t count) {
    (void)size;

    int32_t i;
    for (i = 0; i < count; i ++) {
        ecs_os_free(ptr[i].alloc_value);
    }
}

void EcsName_copy(EcsName *dst, const EcsName *src, size_t size, int32_t count) {
    (void)size;
    
    int32_t i;
    for (i = 0; i < count; i ++) {
        if (dst[i].alloc_value) {
            ecs_os_free(dst[i].alloc_value);
            dst[i].alloc_value = NULL;
        }
        
        if (src[i].alloc_value) {
            dst[i].alloc_value = ecs_os_strdup(src[i].alloc_value);
            dst[i].value = dst[i].alloc_value;
        } else {
            dst[i].alloc_value = NULL;
            dst[i].value = src[i].value;
        }
        dst[i].symbol = src[i].symbol;
    }
}

void EcsName_move(EcsName *dst, EcsName *src, size_t size, int32_t count) {
    memcpy(dst, src, size * ecs_to_size_t(count));
    memset(src, 0, size * ecs_to_size_t(count));
}


/* -- Bootstrapping -- */

#define bootstrap_component(world, table, name)\
    _bootstrap_component(world, table, ecs_typeid(name), #name, sizeof(name),\
        ECS_ALIGNOF(name))

void _bootstrap_component(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t entity,
    const char *id,
    ecs_size_t size,
    ecs_size_t alignment)
{
    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);

    ecs_data_t *data = ecs_table_get_or_create_data(table);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR);

    ecs_column_t *columns = data->columns;
    ecs_assert(columns != NULL, ECS_INTERNAL_ERROR);

    /* Create record in entity index */
    ecs_record_t *record = ecs_eis_get_or_create(world, entity);
    record->table = table;

    /* Insert row into table to store EcsComponent itself */
    int32_t index = ecs_table_append(world, table, data, entity, record, false);
    record->row = index + 1;

    /* Set size and id */
    EcsComponent *c_info = ecs_vector_first(columns[0].data, EcsComponent);
    EcsName *id_data = ecs_vector_first(columns[1].data, EcsName);
    
    c_info[index].size = size;
    c_info[index].alignment = alignment;
    id_data[index].value = &id[ecs_os_strlen("Ecs")]; /* Skip prefix */
    id_data[index].symbol = id;
    id_data[index].alloc_value = NULL;
}

/** Create type for component */
ecs_type_t ecs_bootstrap_type(
    ecs_world_t *world,
    ecs_entity_t entity)
{
    ecs_table_t *table = ecs_table_find_or_create(world, &(ecs_entities_t){
        .array = (ecs_entity_t[]){entity},
        .count = 1
    });

    ecs_assert(table != NULL, ECS_INTERNAL_ERROR);
    ecs_assert(table->type != NULL, ECS_INTERNAL_ERROR);

    return table->type;
}

/** Initialize component table. This table is manually constructed to bootstrap
 * flecs. After this function has been called, the builtin components can be
 * created. 
 * The reason this table is constructed manually is because it requires the size
 * and alignment of the EcsComponent and EcsName components, which haven't been 
 * created yet */
static
ecs_table_t* bootstrap_component_table(
    ecs_world_t *world)
{
    ecs_entity_t entities[] = {ecs_typeid(EcsComponent), ecs_typeid(EcsName), ecs_role(EcsScope, EcsFlecsCore)};
    ecs_entities_t array = {
        .array = entities,
        .count = 3
    };

    ecs_table_t *result = ecs_table_find_or_create(world, &array);
    ecs_data_t *data = ecs_table_get_or_create_data(result);

    /* Preallocate enough memory for initial components */
    data->entities = ecs_vector_new(ecs_entity_t, EcsFirstUserComponentId);
    data->record_ptrs = ecs_vector_new(ecs_record_t*, EcsFirstUserComponentId);

    data->columns = ecs_os_malloc(sizeof(ecs_column_t) * 2);
    ecs_assert(data->columns != NULL, ECS_OUT_OF_MEMORY);

    data->columns[0].data = ecs_vector_new(EcsComponent, EcsFirstUserComponentId);
    data->columns[0].size = sizeof(EcsComponent);
    data->columns[0].alignment = ECS_ALIGNOF(EcsComponent);
    data->columns[1].data = ecs_vector_new(EcsName, EcsFirstUserComponentId);
    data->columns[1].size = sizeof(EcsName);
    data->columns[1].alignment = ECS_ALIGNOF(EcsName);

    result->column_count = 2;
    
    return result;
}

void ecs_bootstrap(
    ecs_world_t *world)
{
    ecs_trace_1("bootstrap core components");
    ecs_log_push();

    /* Create table that will hold components (EcsComponent, EcsName) */
    ecs_table_t *table = bootstrap_component_table(world);
    assert(table != NULL);

    bootstrap_component(world, table, EcsComponent);
    bootstrap_component(world, table, EcsName);

    world->stats.last_component_id = EcsFirstUserComponentId;
    world->stats.last_id = EcsFirstUserEntityId;

    ecs_set_scope(world, EcsFlecsCore);

    ecs_bootstrap_tag(world, EcsModule);
    ecs_bootstrap_tag(world, EcsDisabled);
    ecs_bootstrap_tag(world, EcsWildcard);
    ecs_bootstrap_tag(world, EcsScope);

    ecs_set_lifecycle(world, ecs_typeid(EcsName), &(ecs_lifecycle_t){
        .ctor = (ecs_xtor_t)EcsName_ctor,
        .dtor = (ecs_xtor_t)EcsName_dtor,
        .copy = (ecs_copy_t)EcsName_copy,
        .move = (ecs_move_t)EcsName_move
    });

    /* Initialize scopes */
    ecs_set(world, EcsFlecs, EcsName, {.value = "flecs"});
    ecs_add_id(world, EcsFlecs, EcsModule);
    ecs_set(world, EcsFlecsCore, EcsName, {.value = "core"});
    ecs_add_id(world, EcsFlecsCore, EcsModule);
    ecs_add_id(world, EcsFlecsCore, ecs_role(EcsScope, EcsFlecs));

    ecs_set_scope(world, 0);

    ecs_log_pop();
}
