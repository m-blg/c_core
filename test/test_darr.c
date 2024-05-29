
#include <criterion/criterion.h>
// #define CORE_CORE_IMPL
// #include "core/core.h"
#define CORE_IMPL
#include "core/array.h"

Test(slice, basic) {
// int main() {
    ctx_init_default();
    slice_T(int) sl;
    slice_T(int) sl2;
    slice_new_in_T(int, 3, &g_ctx.global_alloc, &sl);
    slice_new_in_T(int, 3, &g_ctx.global_alloc, &sl2);
    *slice_get_T(int, &sl, 0) = 5;
    *slice_get_T(int, &sl, 1) = 7;
    *slice_get_T(int, &sl, 2) = 9;
    slice_copy_data(&sl, &sl2);
    for_in_range(i, 0, slice_len(&sl)) {
        printf("%d %d\n", *slice_get_T(int, &sl, i), *slice_get_iT(int, &sl2, -i - 1));
    }
    slice_free(&sl, &g_ctx.global_alloc);
    slice_free(&sl2, &g_ctx.global_alloc);
}

// Test(darr, basic)
int main()
{
    // printf("\n===================\nbasic\n");

    ctx_init_default();

    i32_t x = 3;
    darr_t arr; 
    darr_new_cap_in_T(i32_t, 3, &g_ctx.global_alloc, &arr);
    darr_push(&arr, &x);
    darr_push(&arr, &(i32_t){5});
    darr_push(&arr, &(i32_t){7});
    darr_push(&arr, &(i32_t){9});
    darr_i32_dbg_print(arr);
    for_in_range(i, 0, darr_len(arr)) {
        printf("%d %d\n", *darr_get_T(i32_t, arr, i), *darr_get_iT(i32_t, arr, -i - 1));
    }
    darr_free(&arr);
}