/**
 * @file api_defines.h
 * @brief Supporting defines for the public API.
 *
 * This file contains constants / macro's that are typically not used by an
 * application but support the public API, and therefore must be exposed. This
 * header should not be included by itself.
 */

#ifndef FLECS_API_DEFINES_H
#define FLECS_API_DEFINES_H

/* Standard library dependencies */
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

/* Non-standard but required. If not provided by platform, add manually. */
#include <stdint.h>

/* Contains macro's for importing / exporting symbols */
#include "../bake_config.h"

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//// Language support defines
////////////////////////////////////////////////////////////////////////////////

#ifndef FLECS_LEGACY
#include <stdbool.h>
#endif

/* The API uses the native bool type in C++, or a custom one in C */
#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined)
#undef bool
#undef true
#undef false
typedef char bool;
#define false 0
#define true !false
#endif

typedef uint32_t ecs_flags32_t;
typedef uint64_t ecs_flags64_t;

/* Keep unsigned integers out of the codebase as they do more harm than good */
typedef int32_t ecs_size_t;

/** This reserves entity ids for components. Regular entity ids will start after
 * this constant. This affects performance of table traversal, as edges with ids 
 * lower than this constant are looked up in an array, whereas constants higher
 * than this id are looked up in a map. Increasing this value can improve
 * performance at the cost of (significantly) higher memory usage. */
#define ECS_HI_COMPONENT_ID (256) /* Maximum number of components */


////////////////////////////////////////////////////////////////////////////////
//// Reserved ids
////////////////////////////////////////////////////////////////////////////////

/* Builtin component ids */
#define FLECS__EEcsComponent (1)
#define FLECS__EEcsName (2)

/* Builtin tag ids */
#define EcsModule (ECS_HI_COMPONENT_ID + 0)
#define EcsDisabled (ECS_HI_COMPONENT_ID + 1)
#define EcsWildcard (ECS_HI_COMPONENT_ID + 2)

/* Builtin roles */
#define EcsScope (ECS_HI_COMPONENT_ID + 3)

/* Builtin module ids */
#define EcsFlecs (ECS_HI_COMPONENT_ID + 4)
#define EcsFlecsCore (ECS_HI_COMPONENT_ID + 5)

/* Value used to quickly check if component is builtin. This is used to quickly
 * filter out tables with builtin components (for example for ecs_delete) */
#define EcsLastInternalComponentId (ecs_typeid(EcsName))

/* The first user-defined component starts from this id. Ids up to this number
 * are reserved for builtin components */
#define EcsFirstUserComponentId (32)

/* The first user-defined entity starts from this id. Ids up to this number
 * are reserved for builtin components */
#define EcsFirstUserEntityId (ECS_HI_COMPONENT_ID + 32)


////////////////////////////////////////////////////////////////////////////////
//// Entity id macro's
////////////////////////////////////////////////////////////////////////////////

#define ECS_ENTITY_MASK       ((uint64_t)0xFFFFFFFF)
#define ECS_GENERATION_MASK   ((uint64_t)0xFFFF << 32)
#define ECS_GENERATION(e)     ((e & ECS_GENERATION_MASK) >> 32)
#define ECS_GENERATION_INC(e) ((e & ~ECS_GENERATION_MASK) | ((ECS_GENERATION(e) + 1) << 32))


////////////////////////////////////////////////////////////////////////////////
//// Utilities for working with trait identifiers
////////////////////////////////////////////////////////////////////////////////

#define ecs_entity_t_lo(value) ((uint32_t)(value))
#define ecs_entity_t_hi(value) ((uint32_t)((value) >> 32))
#define ecs_entity_t_comb(v1, v2) (((uint64_t)(v2) << 32) + (uint32_t)(v1))


////////////////////////////////////////////////////////////////////////////////
//// Error codes
////////////////////////////////////////////////////////////////////////////////

#define ECS_INTERNAL_ERROR (1)
#define ECS_INVALID_OPERATION (2)
#define ECS_INVALID_PARAMETER (3)
#define ECS_INVALID_ID (4)
#define ECS_INVALID_COMPONENT (5)
#define ECS_OUT_OF_MEMORY (6)
#define ECS_MISSING_OS_API (7)
#define ECS_INCONSISTENT_COMPONENT_ACTION (8)
#define ECS_INVALID_FROM_WORKER (9)


////////////////////////////////////////////////////////////////////////////////
//// Utilities
////////////////////////////////////////////////////////////////////////////////

#define ECS_OFFSET(o, offset) (void*)(((uintptr_t)(o)) + ((uintptr_t)(offset)))
#define ECS_SIZEOF(T) (ecs_size_t)sizeof(T)
#define ECS_ALIGN(size, alignment) (ecs_size_t)((((((size_t)size) - 1) / ((size_t)alignment)) + 1) * ((size_t)alignment))
#define ECS_MAX(a, b) ((a > b) ? a : b)

/* Use alignof in C++, or a trick in C. */
#ifdef __cplusplus
#define ECS_ALIGNOF(T) (int64_t)alignof(T)
#elif defined(_MSC_VER)
#define ECS_ALIGNOF(T) (int64_t)__alignof(T)
#elif defined(__GNUC__)
#define ECS_ALIGNOF(T) (int64_t)__alignof__(T)
#else
#define ECS_ALIGNOF(T) ((int64_t)&((struct { char c; T d; } *)0)->d)
#endif

#if defined(__GNUC__)
#define ECS_UNUSED __attribute__((unused))
#else
#define ECS_UNUSED
#endif

#ifdef __cplusplus
}
#endif

#endif
