#define CORE_IMPL
#include "core/string.h"

void
test1() {
    auto s = S("а3бв");
    rune_t r;
    usize_t l; 
    ASSERT_OK(str_rune_len(s, &l));
    for_in_range(i, 0, l+1) {
        printlnf("%s, %ld, %ld, %d", s.ptr, strlen((char *)s.ptr), str_len(s), i);
        str_next_rune(s, &r, &s);
    }
    // printlnf("%s, %ld", s.ptr, strlen((char *)s.ptr));
    // printlnf("%s, %ld", s2.ptr, strlen((char *)s2.ptr));

    str_t s2;
    str_next_rune(S("3"), &r, &s2);
    printlnf("%d", r);

    // dbgp(slice, &b);
}

void
test2() {
    auto s = "\U00010348";
    printf("%x %x %x %x\n", s[0], s[1], s[2], s[3]);
    auto s2 = S("\U00010348");
    println_fmt(s2);
    rune_t r;
    ASSERT_OK(str_next_rune(s2, &r, nullptr));
    printf("%x\n", r);

    uchar_t cs[4] = {};
    auto s3 = str_from_ptr_len(&cs, 4);
    ASSERT_OK(str_encode_next_rune(s3, r, nullptr));
    printf("%x %x %x %x\n", cs[0], cs[1], cs[2], cs[3]);
    println_fmt(s3);

    // u16_t x = 1 << 10 | 1 << 3;
    // u8_t x2[2] = { 1 << 2 , 1 << 3 };
    // u8_t x3[2] = { 1 << 3 , 1 << 2 };
    // printf("%x %x %x\n", x, *(u16_t *)&x2, *(u16_t *)&x3);
}

void
test3() {
    slice_T(str_t) ss = slice_lit(
        S("\u0024"),
        S("\u0418"),
        S("\u20ac"),
        S("\U00010348")
    );
    uchar_t cs[4] = {};
    auto ostr = str_from_ptr_len(&cs, 4);
    rune_t r;

    for_in_range(i, 0, slice_len(&ss)) {
        auto s = *slice_get_T(str_t, &ss, i);
        ASSERT_OK(str_next_rune(s, &r, nullptr));
        ASSERT_OK(str_encode_next_rune(ostr, r, nullptr));
        println_fmt(S("%s %s"), s, ostr);
    }
}

void
test4() {
    auto istr = S("\u0024" "\u0418" "\u20ac" "\U00010348");
    
    String ostring;
    // ASSERT_OK(string_new_cap_in(str_len(istr), &g_ctx.global_alloc, &ostring));
    ASSERT_OK(string_from_str_in(istr, &g_ctx.global_alloc, &ostring))
    auto ostr = string_to_str(&ostring);
    rune_t r;

    str_t istr_it = istr;
    str_t ostr_it = ostr;
    while (true) {
        auto err = str_next_rune(istr_it, &r, &istr_it);
        if (IS_ERR(err)) {
            ASSERT(err == UTF8_ERROR(EMPTY_STRING));
            break;
        }
        ASSERT_OK(str_encode_next_rune(ostr_it, r, &ostr_it));
    }
    println_fmt(istr);
    println_fmt(ostr);

    string_free(&ostring);
}

int
main() {
    ctx_init_default();
    // test1();
    // test2();
    // test3();
    test4();
}