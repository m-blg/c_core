#define CORE_IMPL
#include "core/list.h"
#include "core/iter.h"


void
test1() {
    list_t list;
    ASSERT_OK(list_new_in(&g_ctx.global_alloc, typeid_of(int), &list));
    ASSERT_OK(list_push(list, &(int) {3}));
    ASSERT_OK(list_push(list, &(int) {5}));
    ASSERT_OK(list_push(list, &(int) {7}));

    for_in_list_T(int, item, list, {
        print_fmt(S("%d\n"), *item);
    })
    for_in_iter_pref(list, x, list_iter(list), {
        print_fmt(S("%d\n"), *(int *)x);
    })
    int *a = nullptr;
    #define f(x) (*(int *)(x) == 5)
    iter_pref_find(list, list_iter(list), f, &a);
    print_fmt(S("%d\n"), *a);

    list_free(&list);
}

int main()
{
    ctx_init_default();

    test1();
}