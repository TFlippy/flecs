
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <base.h>

// Testsuite 'Vector'
void Vector_setup(void);
void Vector_free_empty(void);
void Vector_count(void);
void Vector_count_empty(void);
void Vector_get(void);
void Vector_get_first(void);
void Vector_get_last(void);
void Vector_get_last_from_empty(void);
void Vector_get_last_from_null(void);
void Vector_get_empty(void);
void Vector_get_out_of_bound(void);
void Vector_add_empty(void);
void Vector_add_resize(void);
void Vector_sort_rnd(void);
void Vector_sort_sorted(void);
void Vector_sort_empty(void);
void Vector_sort_null(void);
void Vector_size_of_null(void);
void Vector_set_size_smaller_than_count(void);
void Vector_pop_elements(void);
void Vector_pop_null(void);
void Vector_reclaim(void);
void Vector_grow(void);
void Vector_copy(void);
void Vector_copy_null(void);
void Vector_memory(void);
void Vector_memory_from_null(void);
void Vector_addn_to_null(void);
void Vector_addn_to_0_size(void);
void Vector_addn_one(void);
void Vector_set_min_count(void);
void Vector_set_min_size(void);
void Vector_set_min_size_to_smaller(void);
void Vector_set_size_of_null(void);
void Vector_new_from_array(void);
void Vector_remove_first(void);
void Vector_remove_second(void);
void Vector_remove_last(void);
void Vector_clear(void);
void Vector_assert_size(void);
void Vector_assert_wrong_size(void);

// Testsuite 'Map'
void Map_setup(void);
void Map_count(void);
void Map_count_empty(void);
void Map_set_overwrite(void);
void Map_set_rehash(void);
void Map_set_zero_buckets(void);
void Map_get(void);
void Map_get_all(void);
void Map_get_empty(void);
void Map_get_unknown(void);
void Map_get_from_null(void);
void Map_iter(void);
void Map_iter_empty(void);
void Map_iter_zero_buckets(void);
void Map_remove(void);
void Map_remove_empty(void);
void Map_remove_unknown(void);
void Map_grow(void);
void Map_set_size_0(void);
void Map_clear(void);
void Map_copy(void);
void Map_memory(void);

// Testsuite 'Sparse'
void Sparse_setup(void);
void Sparse_add_1(void);
void Sparse_add_1_to_empty(void);
void Sparse_add_1_chunk_size_1(void);
void Sparse_add_n(void);
void Sparse_add_n_chunk_size_1(void);
void Sparse_remove(void);
void Sparse_remove_first(void);
void Sparse_remove_last(void);
void Sparse_remove_all(void);
void Sparse_remove_all_n_chunks(void);
void Sparse_remove_deleted(void);
void Sparse_remove_future_version(void);
void Sparse_clear_1(void);
void Sparse_clear_empty(void);
void Sparse_clear_n(void);
void Sparse_clear_n_chunks(void);
void Sparse_add_after_clear(void);
void Sparse_memory_null(void);
void Sparse_copy(void);
void Sparse_restore(void);
void Sparse_create_delete(void);
void Sparse_create_delete_2(void);
void Sparse_count_of_null(void);
void Sparse_size_of_null(void);
void Sparse_copy_null(void);
void Sparse_set_id_source(void);
void Sparse_new_ids(void);
void Sparse_last_id(void);
void Sparse_exists(void);
void Sparse_get_any(void);
void Sparse_get_any_no_chunk(void);
void Sparse_recycle_last_unused(void);
void Sparse_get_or_create_after_max(void);
void Sparse_get_or_create_after_max_empty(void);
void Sparse_get_or_create_no_recycle(void);
void Sparse_get_lo_after_create_hi(void);
void Sparse_create_lo_after_create_hi(void);

// Testsuite 'Dense'
void Dense_setup(void);
void Dense_get_or_create(void);
void Dense_get_or_create_exists(void);
void Dense_get_not_exist(void);
void Dense_get_0_size(void);
void Dense_remove(void);
void Dense_remove_not_exists(void);

// Testsuite 'Ptree'
void Ptree_setup(void);
void Ptree_get_first_65k(void);
void Ptree_get_existing(void);
void Ptree_new_index_below_prev(void);
void Ptree_new_index_below_prev_page_size_limit(void);
void Ptree_new_index_above_prev(void);
void Ptree_new_index_above_prev_page_size_limit(void);
void Ptree_iter_tiny(void);
void Ptree_iter(void);

bake_test_case Vector_testcases[] = {
    {
        "free_empty",
        Vector_free_empty
    },
    {
        "count",
        Vector_count
    },
    {
        "count_empty",
        Vector_count_empty
    },
    {
        "get",
        Vector_get
    },
    {
        "get_first",
        Vector_get_first
    },
    {
        "get_last",
        Vector_get_last
    },
    {
        "get_last_from_empty",
        Vector_get_last_from_empty
    },
    {
        "get_last_from_null",
        Vector_get_last_from_null
    },
    {
        "get_empty",
        Vector_get_empty
    },
    {
        "get_out_of_bound",
        Vector_get_out_of_bound
    },
    {
        "add_empty",
        Vector_add_empty
    },
    {
        "add_resize",
        Vector_add_resize
    },
    {
        "sort_rnd",
        Vector_sort_rnd
    },
    {
        "sort_sorted",
        Vector_sort_sorted
    },
    {
        "sort_empty",
        Vector_sort_empty
    },
    {
        "sort_null",
        Vector_sort_null
    },
    {
        "size_of_null",
        Vector_size_of_null
    },
    {
        "set_size_smaller_than_count",
        Vector_set_size_smaller_than_count
    },
    {
        "pop_elements",
        Vector_pop_elements
    },
    {
        "pop_null",
        Vector_pop_null
    },
    {
        "reclaim",
        Vector_reclaim
    },
    {
        "grow",
        Vector_grow
    },
    {
        "copy",
        Vector_copy
    },
    {
        "copy_null",
        Vector_copy_null
    },
    {
        "memory",
        Vector_memory
    },
    {
        "memory_from_null",
        Vector_memory_from_null
    },
    {
        "addn_to_null",
        Vector_addn_to_null
    },
    {
        "addn_to_0_size",
        Vector_addn_to_0_size
    },
    {
        "addn_one",
        Vector_addn_one
    },
    {
        "set_min_count",
        Vector_set_min_count
    },
    {
        "set_min_size",
        Vector_set_min_size
    },
    {
        "set_min_size_to_smaller",
        Vector_set_min_size_to_smaller
    },
    {
        "set_size_of_null",
        Vector_set_size_of_null
    },
    {
        "new_from_array",
        Vector_new_from_array
    },
    {
        "remove_first",
        Vector_remove_first
    },
    {
        "remove_second",
        Vector_remove_second
    },
    {
        "remove_last",
        Vector_remove_last
    },
    {
        "clear",
        Vector_clear
    },
    {
        "assert_size",
        Vector_assert_size
    },
    {
        "assert_wrong_size",
        Vector_assert_wrong_size
    }
};

bake_test_case Map_testcases[] = {
    {
        "count",
        Map_count
    },
    {
        "count_empty",
        Map_count_empty
    },
    {
        "set_overwrite",
        Map_set_overwrite
    },
    {
        "set_rehash",
        Map_set_rehash
    },
    {
        "set_zero_buckets",
        Map_set_zero_buckets
    },
    {
        "get",
        Map_get
    },
    {
        "get_all",
        Map_get_all
    },
    {
        "get_empty",
        Map_get_empty
    },
    {
        "get_unknown",
        Map_get_unknown
    },
    {
        "get_from_null",
        Map_get_from_null
    },
    {
        "iter",
        Map_iter
    },
    {
        "iter_empty",
        Map_iter_empty
    },
    {
        "iter_zero_buckets",
        Map_iter_zero_buckets
    },
    {
        "remove",
        Map_remove
    },
    {
        "remove_empty",
        Map_remove_empty
    },
    {
        "remove_unknown",
        Map_remove_unknown
    },
    {
        "grow",
        Map_grow
    },
    {
        "set_size_0",
        Map_set_size_0
    },
    {
        "clear",
        Map_clear
    },
    {
        "copy",
        Map_copy
    },
    {
        "memory",
        Map_memory
    }
};

bake_test_case Sparse_testcases[] = {
    {
        "add_1",
        Sparse_add_1
    },
    {
        "add_1_to_empty",
        Sparse_add_1_to_empty
    },
    {
        "add_1_chunk_size_1",
        Sparse_add_1_chunk_size_1
    },
    {
        "add_n",
        Sparse_add_n
    },
    {
        "add_n_chunk_size_1",
        Sparse_add_n_chunk_size_1
    },
    {
        "remove",
        Sparse_remove
    },
    {
        "remove_first",
        Sparse_remove_first
    },
    {
        "remove_last",
        Sparse_remove_last
    },
    {
        "remove_all",
        Sparse_remove_all
    },
    {
        "remove_all_n_chunks",
        Sparse_remove_all_n_chunks
    },
    {
        "remove_deleted",
        Sparse_remove_deleted
    },
    {
        "remove_future_version",
        Sparse_remove_future_version
    },
    {
        "clear_1",
        Sparse_clear_1
    },
    {
        "clear_empty",
        Sparse_clear_empty
    },
    {
        "clear_n",
        Sparse_clear_n
    },
    {
        "clear_n_chunks",
        Sparse_clear_n_chunks
    },
    {
        "add_after_clear",
        Sparse_add_after_clear
    },
    {
        "memory_null",
        Sparse_memory_null
    },
    {
        "copy",
        Sparse_copy
    },
    {
        "restore",
        Sparse_restore
    },
    {
        "create_delete",
        Sparse_create_delete
    },
    {
        "create_delete_2",
        Sparse_create_delete_2
    },
    {
        "count_of_null",
        Sparse_count_of_null
    },
    {
        "size_of_null",
        Sparse_size_of_null
    },
    {
        "copy_null",
        Sparse_copy_null
    },
    {
        "set_id_source",
        Sparse_set_id_source
    },
    {
        "new_ids",
        Sparse_new_ids
    },
    {
        "last_id",
        Sparse_last_id
    },
    {
        "exists",
        Sparse_exists
    },
    {
        "get_any",
        Sparse_get_any
    },
    {
        "get_any_no_chunk",
        Sparse_get_any_no_chunk
    },
    {
        "recycle_last_unused",
        Sparse_recycle_last_unused
    },
    {
        "get_or_create_after_max",
        Sparse_get_or_create_after_max
    },
    {
        "get_or_create_after_max_empty",
        Sparse_get_or_create_after_max_empty
    },
    {
        "get_or_create_no_recycle",
        Sparse_get_or_create_no_recycle
    },
    {
        "get_lo_after_create_hi",
        Sparse_get_lo_after_create_hi
    },
    {
        "create_lo_after_create_hi",
        Sparse_create_lo_after_create_hi
    }
};

bake_test_case Dense_testcases[] = {
    {
        "get_or_create",
        Dense_get_or_create
    },
    {
        "get_or_create_exists",
        Dense_get_or_create_exists
    },
    {
        "get_not_exist",
        Dense_get_not_exist
    },
    {
        "get_0_size",
        Dense_get_0_size
    },
    {
        "remove",
        Dense_remove
    },
    {
        "remove_not_exists",
        Dense_remove_not_exists
    }
};

bake_test_case Ptree_testcases[] = {
    {
        "get_first_65k",
        Ptree_get_first_65k
    },
    {
        "get_existing",
        Ptree_get_existing
    },
    {
        "new_index_below_prev",
        Ptree_new_index_below_prev
    },
    {
        "new_index_below_prev_page_size_limit",
        Ptree_new_index_below_prev_page_size_limit
    },
    {
        "new_index_above_prev",
        Ptree_new_index_above_prev
    },
    {
        "new_index_above_prev_page_size_limit",
        Ptree_new_index_above_prev_page_size_limit
    },
    {
        "iter_tiny",
        Ptree_iter_tiny
    },
    {
        "iter",
        Ptree_iter
    }
};

static bake_test_suite suites[] = {
    {
        "Vector",
        Vector_setup,
        NULL,
        40,
        Vector_testcases
    },
    {
        "Map",
        Map_setup,
        NULL,
        21,
        Map_testcases
    },
    {
        "Sparse",
        Sparse_setup,
        NULL,
        37,
        Sparse_testcases
    },
    {
        "Dense",
        Dense_setup,
        NULL,
        6,
        Dense_testcases
    },
    {
        "Ptree",
        Ptree_setup,
        NULL,
        8,
        Ptree_testcases
    }
};

int main(int argc, char *argv[]) {
    ut_init(argv[0]);
    return bake_test_run("base", argc, argv, suites, 5);
}
