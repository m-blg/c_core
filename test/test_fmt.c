#define CORE_IMPL
#include "core/fmt/fmt.h"


// #define VAT_N(_1, _2, N, ...) N
// #define VAT() VAT_N(1, 2, 3, 4, 5)
#define aaa(...) #__VA_ARGS__ 
// #define Tup(T1, T2, T3)             \
// struct {                            \
//     typeof(T1) _1;                  \
//     typeof(T2) _2;                  \
//     typeof(T3) _3;                  \
// }                                   \

// #define tup_lit(v1, v2, v3)             \
// (struct {                            \
//     typeof(v1) _1;                  \
//     typeof(v2) _2;                  \
//     typeof(v3) _3;                  \
// }) {                               \
//     ._1 = v1,                      \
//     ._2 = v2,                      \
//     ._3 = v3,                      \
// }                                  \


// IOError
// dbgp_formattable(Formattable fo) {
//     auto fmt = string_formatter_default(&g_ctx.stdout_sw);
//     ASSERT_OK(formattable_fmt(&fo, &fmt));
    
//     // TODO panic on FmtError, return IO_Error
//     ASSERT_OK(string_formatter_writeln(&fmt, S(KYEL"\ntest\n"aaa(3, A))));
//     TRY(stream_writer_flush(&fmt.target));
// }
#define STRINGIFY_VA(args...) #args
#define VA(...) STRINGIFY_VA(__VA_OPT__(,) __VA_ARGS__)

void
test1() {

    // int x, y, z;
    // Tup(int, str_t, double) t = tup_lit(x, y, z);
    auto b = slice_lit(3, 4, 5);
    // b.el_dbg_fmt = (FmtFn *)i32_dbg_fmt;
    container_dbg_init(i32, &b);
    // i32_t x = slice_len(&b);
    // i32_t x = ;
    i32_t x = *slice_get_T(i32_t, &b, 1);
    // i32_t x = *slice_get_T(i32_t, &b, 1);
    // auto fo = i32_formattable(&x);
    dbgp(i32, &x);
    dbgp(slice, &b);
}
void
test2() {
    // ctx_init_default();

    // int x, y, z;
    // Tup(int, str_t, double) t = tup_lit(x, y, z);
    auto b = slice_lit(3, 4, 5);
    // b.el_dbg_fmt = (FmtFn *)i32_dbg_fmt;
    container_dbg_init(i32, &b);
    container_print_init(i32, &b);
    // i32_t x = slice_len(&b);
    // i32_t x = ;
    i32_t x = *slice_get_T(i32_t, &b, 1);
    // i32_t x = *slice_get_T(i32_t, &b, 1);
    // auto fo = i32_formattable(&x);
    print(i32, &(i32_t){x + 1});
    println(i32, &x);
    // println(slice, &b);
    print(i32, &(i32_t){'A'});
}

#define fobj_prim(val) (_Generic((val), \
        i32_t: i32_formattable,\
        str_t: str_formattable\
    ))(val)\

bool
str_to_u64(str_t s, u64_t *val) {
    register u64_t x = 0;
    for_in_range(i, 0, str_len(s), {
        u8_t c = *str_get_byte(s, i);
        if (c < '0' || c > '9') {
            return false;
        }
        x += (c - '0') * (i+1);
    })
    *val = x;
    return true;
}




// void
// print_fmt(str_t fmt_str, ...) {

// } 

void
test3() {
    // ctx_init_default();

    // int x, y, z;
    // Tup(int, str_t, double) t = tup_lit(x, y, z);
    auto b = slice_lit(3, 4, 5);
    // b.el_dbg_fmt = (FmtFn *)i32_dbg_fmt;
    container_dbg_init(i32, &b);
    container_print_init(i32, &b);
    // i32_t x = slice_len(&b);
    // i32_t x = ;
    i32_t x = *slice_get_T(i32_t, &b, 1);
    // i32_t x = *slice_get_T(i32_t, &b, 1);
    // auto fo = i32_formattable(&x);
    // print(i32, &(i32_t){x + 1});
    // println(i32, &x);

    i64_t x2 = 1002100310041005;
    u32_t ux = -1u;
    // u64_t ulx = -1u;
    u64_t ulx = 10021003100410051006lu;
    
    println_fmt(S(
        "Hi %s!\n"
        "fmt_obj: %v\n"
        "i32: %d\n"
        "u32: %u\n"
        "i64: %ld\n"
        "u64: %lu"
        ), S("there"),
            fmt_obj_pref(i32_dbg, &x),
            x + 3,
            ux,
            x2,
            ulx);
    // println_fmt(S("Hi %v!"), fmt_obj_T(i32_dbg, &x));
    // print_fmt(S("Hi "), 
    //     fobj_T(i32, 3),
    //     fobj_prim(3),
    //     fobj_T(str, S(" text")));
    println_fmt(S(VA("3", "4")));
}

void
test4() {
    println_fmt(S(
            "%d \\% %d = %d\n"
            "%d \\\\ %d \\= %d\n"
        ), 
            8, 3, 8 % 3,
            12, 3, 8 / 3
            );
}

int main() {
    ctx_init_default();
    test1();
    test2();
    test3();
    test4();
}