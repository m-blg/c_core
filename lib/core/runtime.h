// #ifdef CORE_IMPL
// #define CORE_RUNTIME_IMPL
// #endif // CORE_IMPL

#ifndef CORE_RUNTIME_H
#define CORE_RUNTIME_H

#include "core/core.h"
#include "core/io.h"

Allocator g_c_allocator;

OutputFileStream g_stdout_ofs;
#define STDOUT_STREAM_DEFAULT_BUFFER_SIZE 1024

__thread struct {
    Allocator global_alloc;
    // TODO
    void (*raise)(Error);

    StreamWriter stdout_sw;
} g_ctx;

void
ctx_init_default();
void
io_init(Allocator alloc[non_null]);

#endif // CORE_RUNTIME_H

#if CORE_IMPL_GUARD(CORE_RUNTIME)
#define CORE_RUNTIME_I

#define CORE_IO_IMPL
#include "core/io.h"

void
default_raise(Error e) { }

AllocatorError
c_alloc(void *self, usize_t size, usize_t alignment, void **out_ptr) {
    // malloc return pointers with alignof(max_align_t), so alignment adjustments are not required
    // for alignment <= MAX_ALIGNMENT
    if (alignment > MAX_ALIGNMENT) {
        // TODO
        unimplemented();
    }
    void *p = malloc(size);
    if (p == nullptr) {
        RAISE(ALLOCATOR_ERROR(MEM_ALLOC));
    }

    *out_ptr = p;
    return ALLOCATOR_ERROR(OK);
}

AllocatorError
c_resize(void *self, usize_t size, usize_t alignment, void **in_out_ptr) {
    if (alignment > MAX_ALIGNMENT) {
        // TODO
        unimplemented();
    }
    void *p = realloc(*in_out_ptr, size);
    if (p == nullptr) {
        RAISE(ALLOCATOR_ERROR(MEM_ALLOC));
    }

    *in_out_ptr = p;
    return ALLOCATOR_ERROR(OK);
}
void
c_free(void *self, void **in_out_ptr) {
    free(*in_out_ptr);
    *in_out_ptr = nullptr;
}
// typedef struct Dyn_C_Allocator Dyn_C_Allocator;
// struct Dyn_C_Allocator {
//     Allocator_Vtable _vtable;
//     Allocator *data;
// };

Allocator
c_allocator() {
    return (Allocator) {
        ._vtable = (Allocator_Vtable) {
            .alloc = c_alloc,
            .resize = c_resize,
            .free = c_free,
        },
        .data = nullptr,
    };
}

void
io_init(Allocator alloc[non_null]) {
    output_file_stream_new_in(
        stdout, 
        STDOUT_STREAM_DEFAULT_BUFFER_SIZE, 
        alloc, 
        &g_stdout_ofs);
}

void
ctx_init_default() {
    g_c_allocator = c_allocator();
    g_ctx.global_alloc = g_c_allocator;
    g_ctx.raise = default_raise;

    io_init(&g_ctx.global_alloc);
    g_ctx.stdout_sw = output_file_stream_stream_writer(&g_stdout_ofs);
}
#endif // CORE_RUNTIME_IMPL