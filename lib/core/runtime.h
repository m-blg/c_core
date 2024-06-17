// #ifdef CORE_IMPL
// #define CORE_RUNTIME_IMPL
// #endif // CORE_IMPL

#ifndef CORE_RUNTIME_H
#define CORE_RUNTIME_H

#include "core/core.h"
#include "core/io.h"
#include "core/fmt/fmt.h"
#include "core/arena.h"

Allocator g_c_allocator;

OutputFileStream g_stdout_ofs;
OutputFileStream g_stderr_ofs;
#define STDOUT_STREAM_DEFAULT_BUFFER_SIZE 1024

#define CTX_DUMP_BUFFER_SIZE 4096

__thread struct {
    Allocator global_alloc;

    Arena imm_str_arena;
    Allocator imm_str_alloc;
    slice_T(u8_t) dump_buffer;

    // TODO
    void (*raise)(Error);

    StreamWriter stdout_sw;
    StreamWriter stderr_sw;
} g_ctx;

void
ctx_init_default();
void
io_init(Allocator alloc[non_null]);


#define ctx_global_alloc (&g_ctx.global_alloc)

#include "core/type.h"


#define PRIM_COMMON_IMPL(PREF, T) \
u64_t \
PREF##_hash(T *val) { \
    return (u64_t)val; \
} \
bool \
PREF##_eq(T *l, T *r) { \
    return *l == *r; \
} \
void \
PREF##_set(T *l, T *r) { \
    *l = *r; \
}

PRIM_COMMON_IMPL(int, int)
PRIM_COMMON_IMPL(usize_t, usize_t)


TypeInfo g_type_info[] = {
#define TYPE_LIST_ENTRY(T) (TypeInfo) {\
    .size = sizeof(T),\
    .align = alignof(T),\
    .name = S(STRINGIFY(T)),\
    ._vtable = (TypeInfo_VTable) {\
        .fmt = (FmtFn *)T##_fmt,\
        .dbg_fmt = (FmtFn *)T##_dbg_fmt,\
        .eq = (EqFn *)T##_eq,\
        .set = (SetFn *)T##_set,\
        .hash = (HashFn *)T##_hash,\
    },\
}\

    TYPE_LIST
#undef TYPE_LIST_ENTRY
};

#define type_prop(tid, prop) g_type_info[tid].prop
#define type_vt(tid, fn) g_type_info[tid]._vtable.fn
#define type_prop_T(T, prop) g_type_info[typeid_of(T)].prop
#define type_vt_T(T, fn) g_type_info[typeid_of(T)]._vtable.fn

#endif // CORE_RUNTIME_H

#if CORE_IMPL_GUARD(CORE_RUNTIME)
#define CORE_RUNTIME_I

#define CORE_IO_IMPL
#include "core/io.h"
#include "core/string.h"

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
    output_file_stream_new_in(
        stderr, 
        STDOUT_STREAM_DEFAULT_BUFFER_SIZE, 
        alloc, 
        &g_stderr_ofs);
}

void
ctx_init_default() {
    g_c_allocator = c_allocator();
    g_ctx.global_alloc = g_c_allocator;
    ASSERT_OK(arena_init(&g_ctx.imm_str_arena, 4096, &g_ctx.global_alloc));
    g_ctx.imm_str_alloc = arena_allocator(&g_ctx.imm_str_arena);

    ASSERT_OK(slice_new_in_T(u8_t, CTX_DUMP_BUFFER_SIZE, &g_ctx.global_alloc, &g_ctx.dump_buffer));

    g_ctx.raise = default_raise;

    io_init(&g_ctx.global_alloc);
    g_ctx.stdout_sw = output_file_stream_stream_writer(&g_stdout_ofs);
    g_ctx.stderr_sw = output_file_stream_stream_writer(&g_stderr_ofs);
}

void
ctx_deinit() {
    slice_free(&g_ctx.dump_buffer, &g_ctx.global_alloc);
    arena_deinit(&g_ctx.imm_str_arena);
}

#endif // CORE_RUNTIME_IMPL