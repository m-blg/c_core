#pragma once
#define CORE_IO_IMPL

#include "core/arena.c"

/// @param[in] value: T* 
/// @param[in] fmt: StringFormatter* 
#define dbg_print_proc(T, value, fmt) T##_dbg_print(value, fmt)

enum_def(IOError, 
    IO_ERROR_OK,
    IO_ERROR_WRITE,
)
#define IO_ERROR(ERR) ((IOError)IO_ERROR_##ERR)

typedef IOError (StreamWriter_WriteFn)(void *, usize_t, uint8_t[]);
typedef IOError (StreamWriter_FlushFn)(void *);

struct_def(StreamWriter, {
    struct StreamWriter_VTable {
        StreamWriter_WriteFn *write;
        StreamWriter_FlushFn *flush;
    } _vtable;

    void *data;
})

#ifdef CORE_IO_IMPL
INLINE
IOError
stream_writer_write(StreamWriter self[static 1], usize_t data_size, uint8_t data[data_size]) {
    return self->_vtable.write(self->data, data_size, data);
}
INLINE
IOError
stream_writer_flush(StreamWriter self[static 1]) {
    return self->_vtable.flush(self->data);
}
#endif

typedef struct StringLinearBuffer StringLinearBuffer;
struct StringLinearBuffer {
    Arena arena;
};

#ifdef CORE_IO_IMPL
// Stream Writer Trait
IOError
string_linear_buffer_write(StringLinearBuffer self[static 1], usize_t data_size, uint8_t data[data_size]) {
    return arena_allocator_alloc(&self->arena, data_size, nullptr);
}
IOError
string_linear_buffer_flush(StreamWriter self[static 1]) {
    unimplemented();
}

StreamWriter
string_linear_buffer_stream_writer(StringLinearBuffer self[static 1]) {
    return (StreamWriter) {
        ._vtable = (StreamWriter_VTable) {
            .write = string_linear_buffer_write,
            .flush = string_linear_buffer_flush,
        },
        .data = self,
    };
}
#endif

typedef struct StringRingBuffer StringRingBuffer;

typedef struct OutputFileStream OutputFileStream;
struct OutputFileStream {
    ring_buff_T(CharUTF8) buffer;
    File *file;
};

IOError
output_file_stream_write(OutputFileStream self[static 1], usize_t data_size, uint8_t data[data_size]);
IOError
output_file_stream_flush(OutputFileStream self[static 1]);

#ifdef CORE_IO_IMPL
IOError
output_file_stream_write(OutputFileStream self[static 1], usize_t data_size, uint8_t data[data_size]) {
    while (ring_buff_rest_size(&self->buffer) < data_size) {
        // TODO
        ring_buff_write_slice(data, data_size)
        TRY(output_file_stream_flush(self));
    }
    return IO_ERROR(OK);
}
IOError
output_file_stream_flush(OutputFileStream self[static 1]) {
    if (self->buffer.len == 0) {
        return IO_ERROR(OK);
    }

    usize_t written = fwrite(self->buffer.ptr, self->buffer.el_size, self->buffer.len, self->file);
    if (written < self->buffer.len) {
        return IO_ERROR(WRITE);
    }
    return IO_ERROR(OK);
}
#endif


OutputFileStream g_stdout_stream;
#define STDOUT_STREAM_DEFAULT_BUFFER_SIZE 1024

#ifdef CORE_IO_IMPL
void
io_init() {
    ring_buffer_T(CharUTF8) buffer;
    ring_buffer_new_in(sizeof(char), 
        STDOUT_STREAM_DEFAULT_BUFFER_SIZE, 
        g_ctx.global_alloc, 
        &buffer);
    g_stdout_stream = (OutputFileStream) {
        .buffer = buffer,
        .file = stdout,
    };
}
#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
