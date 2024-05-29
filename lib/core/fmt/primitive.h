#include "core/impl_guards.h"

#if CORE_HS_GUARD(CORE_FMT, PRIMITIVE)
#define CORE_FMT_H
#define CORE_FMT_PRIMITIVE_H

dbg_fmt_proc_def(i32, i32_t, i, fmt, {
    constexpr usize_t size = 64;
    uchar_t buffer[size];
    auto len = snprintf((char *)buffer, size, "%d", *i);
    auto s = ((str_t) {
        .ptr = buffer,
        .byte_len = (usize_t)len,
    });

    TRY(string_formatter_write(fmt, s));
    return FMT_ERROR(OK);
})

// FmtError
// i32_fmt(const i32_t i[non_null], StringFormatter fmt[non_null], void *) {
//     constexpr usize_t size = 64;
//     uchar_t buffer[size];
//     auto len = snprintf((char *)buffer, size, "%d", *i);
//     auto s = (str_t) {
//         .ptr = buffer,
//         .byte_len = (usize_t)len,
//     };

//     TRY(string_formatter_write(fmt, s));
//     return FMT_ERROR(OK);
// }

#define T_FMT_IMPL(NAME, T, PRINTF_STR) \
FmtError \
NAME(const T i[non_null], StringFormatter fmt[non_null], void *) { \
    constexpr usize_t size = 64; \
    uchar_t buffer[size]; \
    auto len = snprintf((char *)buffer, size, PRINTF_STR, *i); \
    auto s = (str_t) { \
        .ptr = buffer, \
        .byte_len = (usize_t)len, \
    }; \
\
    TRY(string_formatter_write(fmt, s)); \
    return FMT_ERROR(OK); \
}

#define NUM_FMT_DBG_FMT_IMPL(PREF, T, PRINTF_STR) \
T_FMT_IMPL(PREF##_fmt, T, PRINTF_STR) \
T_FMT_IMPL(PREF##_dbg_fmt, T, PRINTF_STR)

NUM_FMT_DBG_FMT_IMPL(i32_t, i32_t, "%d")
NUM_FMT_DBG_FMT_IMPL(usize_t, usize_t, "%ld")
NUM_FMT_DBG_FMT_IMPL(int, int, "%d")

// FmtError
// i32_fmt(const i32_t i[non_null], StringFormatter fmt[non_null], void *) {
//     constexpr usize_t size = 64;
//     uchar_t buffer[size];
//     auto len = snprintf((char *)buffer, size, "%d", *i);
//     auto s = (str_t) {
//         .ptr = buffer,
//         .byte_len = (usize_t)len,
//     };

//     TRY(string_formatter_write(fmt, s));
//     return FMT_ERROR(OK);
// }
// FmtError
// _i32_dbg_fmt(void *i, StringFormatter fmt[non_null]) {
//     return i32_dbg_fmt((i32_t *)i, fmt);
// }
// #define formattable_from_fn_data(fmt_fn, _data)   
//     (Formattable) {                               
//         ._vtable = (Formattable_VTable) {         
//             .fmt = (fmt_fn),                      
//         },                                        
//         .data = (void *)(_data),                  
//     }                                             

// INLINE
// Formattable
// i32_formattable(const i32_t i[non_null]) {
//     return formattable_from_fn_data(_i32_dbg_fmt, i);
// } 

#undef CORE_FMT_H
#endif // CORE_FMT_PRIMITIVE_H