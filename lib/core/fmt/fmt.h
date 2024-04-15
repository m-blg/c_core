#include "core/impl_guards.h"

#ifdef CORE_FMT_IMPL
#define CORE_FMT_FORMATTER_IMPL
#define CORE_FMT_FORMATTABLE_IMPL
#endif // CORE_FMT_IMPL




// #if CORE_DS_GUARD(CORE_FMT, FORMATTER)
// #define CORE_FMT_D
// #define CORE_FMT_FORMATTER_D


// #undef CORE_FMT_D
// #endif // CORE_FMT_DECL






#if CORE_HS_GUARD(CORE_FMT, FORMATTER)
#define CORE_FMT_H
#define CORE_FMT_FORMATTER_H

#include "core/core.h"

enum_decl(FmtError);
struct_decl(StringFormatter);

typedef FmtError (FmtFn)(void *, StringFormatter *);

#define FMT_ERROR(ERR) ((FmtError)FMT_ERROR_##ERR)

struct_decl(StreamWriter);
struct_decl(str_t);

INLINE
StringFormatter
string_formatter_default(StreamWriter *sw);

INLINE
void 
string_formatter_init_default(StringFormatter *self, StreamWriter *sw);

FmtError
string_formatter_write(StringFormatter *fmt, const str_t s);
FmtError
string_formatter_writeln(StringFormatter *fmt, const str_t s);


struct_decl(Formattable)
struct_decl(Formattable_VTable)

FmtError 
formattable_fmt(Formattable *self, StringFormatter *fmt);



#include "core/io.h"
#include "core/string.h"


enum_def(FmtError,
    FMT_ERROR_OK,
    FMT_ERROR_ERROR,
)

struct_def(StringFormatter, {
    usize_t pad_level;
    str_t pad_string;

    StreamWriter target;
})


#define container_dbg_init(prefix, self) {(self)->el_dbg_fmt = (FmtFn *)prefix##_dbg_fmt;}
#define container_print_init(prefix, self) {(self)->el_fmt = (FmtFn *)prefix##_dbg_fmt;}


#define formattable_dbg_fmt(fo) formattable_fmt(fo)
// dbg_fmt_pn(ns) ns##_dbg_fmt

#define dbgp(___prefix, val) {                                                     \
    auto fmt = string_formatter_default(&g_ctx.stdout_sw);                      \
    /* auto fo = prefix##_formattable(&(val));  */                                   \
    /* ASSERT_OK(formattable_fmt(&fo, &fmt));   */                                   \
    ASSERT_OK(___prefix##_dbg_fmt((val), &fmt));                                      \
                                                                                \
    /* TODO panic on FmtError, return IO_Error */                               \
    /*ASSERT_OK(string_formatter_writeln(&fmt, S(KYEL"\ntest\n"aaa(3, A))));*/      \
    ASSERT_OK(string_formatter_write(&fmt, S("\n")));      \
    ASSERT_OK(stream_writer_flush(&fmt.target));                                      \
}                                                                               \

#define print(___prefix, val) {                                                     \
    auto fmt = string_formatter_default(&g_ctx.stdout_sw);                      \
    ASSERT_OK(___prefix##_fmt((val), &fmt));                                      \
    ASSERT_OK(stream_writer_flush(&fmt.target));                                      \
}                                                                               \

#define println(___prefix, val) {                                                     \
    auto fmt = string_formatter_default(&g_ctx.stdout_sw);                      \
    ASSERT_OK(___prefix##_fmt((val), &fmt));                                      \
    ASSERT_OK(string_formatter_write(&fmt, S("\n")));      \
    ASSERT_OK(stream_writer_flush(&fmt.target));                                      \
}


FmtError
string_formatter_fmt(StringFormatter *fmt, str_t fmt_str, ...);

#define print_fmt(fmt_str, ...) {                                \
    auto fmt = string_formatter_default(&g_ctx.stdout_sw);       \
    ASSERT_OK(string_formatter_fmt(&fmt, fmt_str, __VA_ARGS__)); \
    ASSERT_OK(stream_writer_flush(&fmt.target));                 \
}                                                                     
#define println_fmt(fmt_str, ...) {                                \
    auto fmt = string_formatter_default(&g_ctx.stdout_sw);       \
    ASSERT_OK(string_formatter_fmt(&fmt, fmt_str, __VA_ARGS__)); \
    ASSERT_OK(string_formatter_write(&fmt, S("\n")));      \
    ASSERT_OK(stream_writer_flush(&fmt.target));                 \
}                                                                     


#undef CORE_FMT_H
#endif // CORE_FMT_FORMATTER_H








#if CORE_IS_GUARD(CORE_FMT, FORMATTER)
#define CORE_FMT_FORMATTER_I

INLINE
StringFormatter
string_formatter_default(StreamWriter *sw) {
    return (StringFormatter) {
        .pad_level = 0,
        .pad_string = S("    "),
        .target = *sw,
    };
}

INLINE
void 
string_formatter_init_default(StringFormatter *self, StreamWriter *sw) {
    *self = string_formatter_default(sw);
}

FmtError
string_formatter_write(StringFormatter *fmt, const str_t s) {
    for_in_range(_, 0, fmt->pad_level, {
        if (stream_writer_write(&fmt->target, 
            fmt->pad_string.byte_len, 
            fmt->pad_string.ptr) != IO_ERROR(OK) ) 
        {
            return FMT_ERROR(ERROR);
        }
    })
    if (stream_writer_write(&fmt->target, s.byte_len, s.ptr) != IO_ERROR(OK)) {
        return FMT_ERROR(ERROR);
    }
    return FMT_ERROR(OK);
}
FmtError
string_formatter_writeln(StringFormatter *fmt, const str_t s) {
    TRY(string_formatter_write(fmt, s));
    auto r = stream_writer_write(&fmt->target, 1, (uchar_t *)"\n");
    if (IS_ERR(r)) {
        return FMT_ERROR(ERROR);
    }
    return FMT_ERROR(OK);
}

#include <stdarg.h>

FmtError
string_formatter_fmt(StringFormatter *fmt, str_t fmt_str, ...) {
    va_list args;
    va_start(args, fmt_str);

    str_t s = (str_t) {
        .ptr = fmt_str.ptr,
        .byte_len = 0,
    };
    rune_t r;
    while (str_len(fmt_str) > 0) {
        ASSERT_OK(str_next_rune(fmt_str, &r, &fmt_str));
        if (r == '%') {
            if (str_len(s) > 0) {
                TRY(string_formatter_write(fmt, s));
                // s = {0};
            }
            ASSERT_OK(str_next_rune(fmt_str, &r, &fmt_str));
            switch (r)
            {
            case 's':
                s = va_arg(args, str_t);
                TRY(string_formatter_write(fmt, s));
                break;
            case 'v':
                auto fo = va_arg(args, Formattable);
                TRY(formattable_fmt(&fo, fmt));
                break;
            
            default:
                return FMT_ERROR(ERROR);
                unreacheble();
                break;
            }

            s = (str_t) {
                .ptr = fmt_str.ptr,
                .byte_len = 0,
            };
        } else {
            s.byte_len += 1;
        }
    }
    if (str_len(s) > 0) {
        TRY(string_formatter_write(fmt, s));
    }

    va_end(args);
    return FMT_ERROR(OK);
}

#endif // CORE_FMT_FORMATTER_IMPL









#if CORE_HS_GUARD(CORE_FMT, FORMATTABLE)
#define CORE_FMT_H
#define CORE_FMT_FORMATTABLE_H

struct_def(Formattable_VTable, {
    FmtFn *fmt;
})
struct_def(Formattable, {
    Formattable_VTable _vtable;
    void *data;
})

#ifdef DBG_PRINT
/// @param[in] value: T* 
/// @param[in] fmt: StringFormatter* 
#define dbg_print_proc(T, value, fmt) T##_dbg_print(value, fmt)

#endif

#undef CORE_FMT_H
#endif // CORE_FMT_FORMATTABLE_H






#if CORE_IS_GUARD(CORE_FMT, FORMATTABLE)
#define CORE_FMT_FORMATTABLE_I
FmtError 
formattable_fmt(Formattable *self, StringFormatter *fmt) {
    return self->_vtable.fmt(self->data, fmt);
}
#endif // CORE_FMT_FORMATTABLE_I

#include "core/fmt/primitive.h"
