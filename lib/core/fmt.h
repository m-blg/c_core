#ifdef CORE_IMPL
#define CORE_FMT_IMPL
#endif // CORE_IMPL

#ifdef CORE_FMT_IMPL
#undef CORE_FMT_IMPL

#define CORE_FMT_FORMATTER_IMPL
#define CORE_FMT_FORMATTABLE_IMPL

#endif // CORE_ARRAY_IMPL

#ifndef CORE_FMT_FORMATTER_H
#define CORE_FMT_FORMATTER_H

#include "core/core.h"
#include "core/string.h"
#include "core/io.h"


enum_def(FmtError,
    FMT_ERROR_OK,
    FMT_ERROR_ERROR,
)
#define FMT_ERROR(ERR) ((FmtError)FMT_ERROR_##ERR)

struct_def(StringFormatter, {
    usize_t pad_level;
    str_t pad_string;

    StreamWriter target;
})

#endif // CORE_FMT_FORMATTER_H

#if defined(CORE_FMT_FORMATTER_IMPL) && !defined(CORE_FMT_FORMATTER_I)
#define CORE_FMT_FORMATTER_I
void 
string_formatter_init(StringFormatter self[static 1]) {
    *self = (StringFormatter) {
        .pad_level = 0,
    };
}

FmtError
string_formatter_write(StringFormatter fmt[static 1], str_t s) {
    for_in_range(_, 0, fmt->pad_level, {
        stream_writer_write(&fmt->target, fmt->pad_string);
    });
    stream_writer_write(&fmt->target, s);
    return FMT_ERROR(OK);
}
FmtError
string_formatter_writeln(StringFormatter fmt[static 1], str_t s) {
    for_in_range(_, 0, fmt->pad_level, {
        if (stream_writer_write(&fmt->target, fmt->pad_string)) {
            return FMT_ERROR(ERROR);
        }
    });
    stream_writer_writeln(&fmt->target, s);
    return FMT_ERROR(OK);
}
#endif // CORE_FMT_FORMATTER_IMPL

#ifndef CORE_FMT_FORMATTABLE_H
#define CORE_FMT_FORMATTABLE_H

typedef FmtError (FmtFn)(void *, StringFormatter *);

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

#endif // CORE_FMT_FORMATTABLE_H

#if defined(CORE_FMT_FORMATTABLE_IMPL) && !defined(CORE_FMT_FORMATTABLE_I)
#define CORE_FMT_FORMATTABLE_I
FmtError 
formattable_fmt(Formattable *self, StringFormatter *fmt) {
    return self->_vtable.fmt(self->data, fmt);
}
#endif


