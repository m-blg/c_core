#define CORE_IMPL
#include "core/string.h"

int
main() {
    ctx_init_default();

    auto s = S("а3бв");
    rune_t r;
    usize_t l; 
    ASSERT_OK(str_rune_len(s, &l));
    for_in_range(i, 0, l+1, {
        printlnf("%s, %ld, %ld, %d", s.ptr, strlen((char *)s.ptr), str_len(s), i);
        str_next_rune(s, &r, &s);
    })
    // printlnf("%s, %ld", s.ptr, strlen((char *)s.ptr));
    // printlnf("%s, %ld", s2.ptr, strlen((char *)s2.ptr));

    str_t s2;
    str_next_rune(S("3"), &r, &s2);
    printlnf("%d", r);

    // dbgp(slice, &b);
}