
#define CORE_IMPL
#include "core/hashmap.h"



void
test1() {
    hashmap_T(str_t, int) map;
    ASSERT_OK(hashmap_new_cap_in(
        typeid_of(str_t), typeid_of(int),
        16, &g_ctx.global_alloc, &map));
    
    hashmap_set(&map, &S("a"), &(int){3});
    hashmap_set(&map, &S("a"), &(int){4});
    hashmap_set(&map, &S("b"), &(int){5});
    hashmap_set(&map, &S("b1"), &(int){6});
    hashmap_set(&map, &S("c"), &(int){7});
    hashmap_set(&map, &S("c2"), &(int){72});
    hashmap_set(&map, &S("d"), &(int){7});
    hashmap_set(&map, &S("d2"), &(int){72});
    hashmap_set(&map, &S("e"), &(int){7});
    hashmap_set(&map, &S("e2"), &(int){72});
    auto a = *hashmap_get_T(int, map, &S("a"));
    auto b = *hashmap_get_T(int, map, &S("b"));
    auto b1 = *hashmap_get_T(int, map, &S("b1"));
    auto c = *hashmap_get_T(int, map, &S("c"));
    auto c2 = *hashmap_get_T(int, map, &S("c2"));
    print_fmt(S("a: %d, "
                "b: %d, "
                "b1: %d, "
                "c: %d, "
                "c2: %d, "
                "\n"), a, b, b1, c, c2);
    
    hashmap_free(&map);
}

int 
main() {
    ctx_init_default();

    test1();
}