
#define CORE_IMPL
#include "core/hashmap.h"

u32_t
str_p_hash(str_t *self) {
    str_t _self = *self;
    u32_t hash = 2166136261u;
    for_in_range(i, (usize_t)0, str_len(_self)) {
        hash ^= (u8_t)_self.ptr[i];
        hash *= 16777619;
    }
    return hash;
}

bool
str_p_eq(str_t *str1, str_t *str2) {
    return str_eq(*str1, *str2);
}

void
str_p_set(str_t *l, str_t *r) {
    *l = *r;
}
void
int_set(int *l, int *r) {
    *l = *r;
}


void
test1() {
    hashmap_T(str_t, int) map;
    ASSERT_OK(hashmap_new_cap_in(
        sizeof(str_t), alignof(str_t),
        (HashFn *)str_p_hash, (EqFn *)str_p_eq, (SetFn *)str_p_set,
        sizeof(int), alignof(int),
        (SetFn *)int_set,
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