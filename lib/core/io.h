#include "core/impl_guards.h"


enum_decl(IOError)

#if CORE_HEADER_GUARD(CORE_IO)
#define CORE_IO_H

#include "core/array.h"
#include "core/string.h"


enum_def(IOError, 
    IO_ERROR_OK,
    IO_ERROR_WRITE,
)
#define IO_ERROR(ERR) ((IOError)IO_ERROR_##ERR)

typedef IOError (StreamWriter_WriteFn)(void *, usize_t, uint8_t[]);
typedef IOError (StreamWriter_FlushFn)(void *);

struct_def(StreamWriter_VTable, {
    StreamWriter_WriteFn *write;
    StreamWriter_FlushFn *flush;
})
struct_def(StreamWriter, {
    StreamWriter_VTable _vtable;

    void *data;
})


// I don't need a ring buffer here
struct_def(OutputFileStream, {
    slice_T(u8_t) buffer;
    void *b_cursor;
    void *e_cursor;
    FILE *file;
})
struct_def(OutputStringStream, {
    slice_T(u8_t) buffer;
    void *b_cursor;
    void *e_cursor;
    FILE *file;
})

#define output_file_stream_reset_cursors(self) ((self)->b_cursor = (self)->e_cursor = (self)->buffer.ptr)
#define output_file_stream_pending_len(self) ((usize_t)(self)->e_cursor - (usize_t)(self)->b_cursor)
#define output_file_stream_rest_size(self) ((usize_t)slice_end(&(self)->buffer) - (usize_t)(self)->e_cursor)

IOError
output_file_stream_write(OutputFileStream self[non_null], usize_t data_size, u8_t data[data_size]);
IOError
output_file_stream_flush(OutputFileStream self[non_null]);

AllocatorError
output_file_stream_new_in(FILE *ofile, usize_t buffer_size, Allocator alloc[non_null], OutputFileStream *out_self);
IOError
output_file_stream_write(OutputFileStream self[non_null], usize_t data_size, u8_t data[data_size]);
IOError
output_file_stream_flush(OutputFileStream self[non_null]);
StreamWriter
output_file_stream_stream_writer(OutputFileStream self[non_null]);


IOError
output_string_stream_write(String self[non_null], usize_t data_size, u8_t data[data_size]);
IOError
output_string_stream_flush(String self[non_null]);

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#endif // CORE_IO_H

// typedef struct StringLinearBuffer StringLinearBuffer;
// struct StringLinearBuffer {
//     Arena arena;
// };

// #ifdef CORE_IO_IMPL
// // Stream Writer Trait
// IOError
// string_linear_buffer_write(StringLinearBuffer self[non_null], usize_t data_size, uint8_t data[data_size]) {
//     return arena_allocator_alloc(&self->arena, data_size, nullptr);
// }
// IOError
// string_linear_buffer_flush(StreamWriter self[non_null]) {
//     unimplemented();
// }

// StreamWriter
// string_linear_buffer_stream_writer(StringLinearBuffer self[non_null]) {
//     return (StreamWriter) {
//         ._vtable = (StreamWriter_VTable) {
//             .write = string_linear_buffer_write,
//             .flush = string_linear_buffer_flush,
//         },
//         .data = self,
//     };
// }
// #endif

// #define CORE_IO_IMPL
#if CORE_IMPL_GUARD(CORE_IO)
#define CORE_IO_I

// #define CORE_ARRAY_IMPL
// #include "core/array.h"

// StreamWriter

INLINE
IOError
stream_writer_write(StreamWriter self[non_null], usize_t data_size, uint8_t data[data_size]) {
    return self->_vtable.write(self->data, data_size, data);
}
INLINE
IOError
stream_writer_flush(StreamWriter self[non_null]) {
    return self->_vtable.flush(self->data);
}

// output_file_stream

AllocatorError
output_file_stream_new_in(FILE *ofile, usize_t buffer_size, Allocator alloc[non_null], OutputFileStream *out_self) {
    out_self->file = ofile;
    TRY(slice_new_in_T(u8_t, buffer_size, alloc, &out_self->buffer));
    out_self->b_cursor = out_self->buffer.ptr;
    out_self->e_cursor = out_self->buffer.ptr;
    return ALLOCATOR_ERROR(OK);
}

IOError
output_file_stream_write(OutputFileStream self[non_null], usize_t data_size, u8_t data[data_size]) {
    // while (ring_buff_rest_size(&self->buffer) < data_size) {
    //     // TODO
    //     ring_buff_write_slice(data, data_size)
    //     TRY(output_file_stream_flush(self));
    // }
    
    while (output_file_stream_rest_size(self) < data_size) {
        // TODO
        // ring_buff_write_slice(data, data_size)
        auto rest_size = output_file_stream_rest_size(self);
        memcpy(self->e_cursor, data, rest_size);
        self->e_cursor = ptr_shift(self->e_cursor, rest_size);
        data_size -= rest_size;
        data += rest_size;
        TRY(output_file_stream_flush(self));
    }
    if (data_size == 0) {
        return IO_ERROR(OK);
    }
    memcpy(self->e_cursor, data, data_size);
    self->e_cursor = ptr_shift(self->e_cursor, data_size);

    return IO_ERROR(OK);
}
IOError
output_file_stream_flush(OutputFileStream self[non_null]) {
    auto len = output_file_stream_pending_len(self);
    if (len == 0) {
        return IO_ERROR(OK);
    }

    usize_t written = fwrite(self->b_cursor, self->buffer.el_size, len, self->file);
    if (written < len) {
        self->b_cursor = ptr_shift(self->b_cursor, written);
        return IO_ERROR(WRITE);
    }
    fflush(self->file);
    output_file_stream_reset_cursors(self);

    return IO_ERROR(OK);
}


// IOError
// _output_file_stream_write(void *self, usize_t data_size, u8_t data[data_size]) {
//     return output_file_stream_write((OutputFileStream *)self, data_size, data);
// }
// IOError
// _output_file_stream_flush(void *self) {
//     return output_file_stream_flush((OutputFileStream *)self);
// }
StreamWriter
output_file_stream_stream_writer(OutputFileStream self[non_null]) {
    return (StreamWriter) {
        ._vtable = (StreamWriter_VTable) {
            .write = (StreamWriter_WriteFn *)output_file_stream_write,
            .flush = (StreamWriter_FlushFn *)output_file_stream_flush,
        },
        .data = (void *)self,
    };
}


IOError
output_string_stream_write(String self[non_null], usize_t data_size, u8_t data[data_size]) {
    ASSERT_OK(string_reserve_cap(self, data_size));
    ASSERT_OK(string_append_str(self, (str_t) {.ptr = data, .byte_len = data_size}));
    return IO_ERROR(OK);
}
IOError
output_string_stream_flush(String self[non_null]) {
    return IO_ERROR(OK);
}
StreamWriter
string_stream_writer(String self[non_null]) {
    return (StreamWriter) {
        ._vtable = (StreamWriter_VTable) {
            .write = (StreamWriter_WriteFn *)output_string_stream_write,
            .flush = (StreamWriter_FlushFn *)output_string_stream_flush,
        },
        .data = (void *)self,
    };
}

#endif // CORE_IO_IMPL