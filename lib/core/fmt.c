#pragma once

#include "core/core.c"
#include "core/string.c"
#include "core/io.c"


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

typedef FmtError (FmtFn)(void *, StringFormatter *);

struct_def(Formattable_VTable, {
    FmtFn *fmt;
})
struct_def(Formattable, {
    Formattable_VTable _vtable;
    void *data;
})

FmtError 
formattable_fmt(Formattable *self, StringFormatter *fmt) {
    return self->_vtable.fmt(self->data, fmt);
}

#ifdef DBG_PRINT

#endif

