#include "private_api.h"


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
