/**
 * @file api_defines.h
 * @brief Supporting types for the public API.
 *
 * This file containstypes that are typically not used by an application but 
 * support the public API, and therefore must be exposed. This header should not
 * be included by itself.
 */

#ifndef FLECS_API_TYPES_H
#define FLECS_API_TYPES_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//// Opaque types
////////////////////////////////////////////////////////////////////////////////

typedef struct ecs_table_t ecs_table_t;


////////////////////////////////////////////////////////////////////////////////
//// Non-opaque types
////////////////////////////////////////////////////////////////////////////////

typedef struct ecs_record_t {
    ecs_table_t *table;  /* Identifies a type (and table) in world */
    int32_t row;         /* Table row of the entity */
} ecs_record_t;

/** Array of entity ids that, other than a type, can live on the stack */
typedef struct ecs_entities_t {
    ecs_entity_t *array;    /**< An array with entity ids */
    int32_t count;          /**< The number of entities in the array */
} ecs_entities_t;

typedef struct ecs_world_info_t {
    ecs_entity_t last_component_id;   /**< Last issued component entity id */
    ecs_entity_t last_id;             /**< Last issued entity id */
} ecs_world_info_t;


////////////////////////////////////////////////////////////////////////////////
//// Function types
////////////////////////////////////////////////////////////////////////////////

/** Constructor/destructor. Used for initializing / deinitializing components. */
typedef void (*ecs_xtor_t)(
    void *ptr,
    size_t size,
    int32_t count);

/** Copy is invoked when a component is copied into another component. */
typedef void (*ecs_copy_t)(
    void *dst_ptr,
    const void *src_ptr,
    size_t size,
    int32_t count);

/** Move is invoked when a component is moved to another component. */
typedef void (*ecs_move_t)(
    void *dst_ptr,
    void *src_ptr,
    size_t size,
    int32_t count);

#ifdef __cplusplus
}
#endif

#endif
