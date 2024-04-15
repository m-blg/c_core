
#include <criterion/criterion.h>
#include "core/list.c"



Test(list, basic)
// int main()
{
    printf("start""\n");

    ctx_init_default();

    ListVNS list;
    list_ves_init(&list, g_ctx.global_alloc);
    list_ves_dbg_print(int, &list);
}