#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef uint64_t usize_t;
typedef uint64_t isize_t;
typedef uint8_t uchar_t;
// typedef uchar_t bool;


typedef enum {
    ERROR_OK,
    ERROR_MEM_ALLOC,
    ERROR_STOP_ITERATOR,
    // ERROR_,
    ERROR_COUNT
} Error;

// __attribute__ ((noreturn))
[[noreturn]]
void 
unimplemented()  { 
    perror("unimplemented");
    exit(1); // TODO: exit callback
}

[[noreturn]]
void 
unreacheble()  { 
    perror("unreacheble");
    exit(1); 
}

// typedef Error (*ExceptionHandler)(Error e);

// #define RAISE(e) _ctx.raise((e))

#define TRY(expr) {             \
    auto e = (expr);            \
    if ((isize_t)e != 0) {      \
        return _ctx.raise(e);   \
    }                           \
  }                             \

void print_stack_trace() {} // TODO(mbgl)
// constexpr int foo() {return 3;}

#define ASSERT(expr)                         \
    if (!(expr)) {                           \
        panic();                             \
    }

#define ASSERT_OK(expr)                      \
    if ((isize_t)(expr) != 0) {                \
        panic();                             \
    }

// design decisions:
//  ergonomics first, then 'speed'

// null ptr error encoding
// typedef void *(*AllocatorAllocFn)(Allocator*, usize_t, usize_t);
// typedef bool (*AllocatorResizeFn)(Allocator*, void *, usize_t, usize_t);
// typedef void (*AllocatorFreeFn)(Allocator*, void *);

// interface
typedef Error (*AllocatorAllocFn)(void*, usize_t, void**);
typedef Error (*AllocatorResizeFn)(void*, void**, usize_t);
typedef void (*AllocatorFreeFn)(void*, void **);

typedef struct {
    struct Allocator_Vtable_s {
        AllocatorAllocFn alloc;
        AllocatorResizeFn resize;
        AllocatorFreeFn free;
    } _vtable;
    // uint8_t data[];
} Allocator;

__thread struct {
    Allocator *global_alloc;
    Error (*raise)(Error);
} _ctx;

Error
allocator_alloc(Allocator* self, usize_t size, void **out_ptr) {
    return self->_vtable.alloc(self, size, out_ptr);
}
Error
allocator_resize(Allocator* self, void **ptr, usize_t size) {
    return self->_vtable.resize(self, ptr, size);
}
void 
allocator_free(Allocator* self, void **ptr) {
    return self->_vtable.free(self, ptr);
}

Error
allocator_alloc_z(Allocator* self, usize_t size, void **out_ptr) {
    TRY(self->_vtable.alloc(self, size, out_ptr));
    memset((void*)*out_ptr, 0, size);
    return ERROR_OK;
}

#define allocator_alloc_n(self, T, count, out_ptr) \
    allocator_alloc(self, sizeof(T) * count, out_ptr)

#define allocator_alloc_zn(self, T, count, out_ptr) \
    allocator_alloc_z(self, sizeof(T) * count, out_ptr)


// TODO(mblg): invoke gdb here
void panic() { 
    print_stack_trace();                     \
    exit(1); 
}


Error
c_alloc(void *self, usize_t size, void **out_ptr) {
    void *p = malloc(size);
    if (p == NULL) {
        return _ctx.raise(ERROR_MEM_ALLOC);
    }

    *out_ptr = p;
    return ERROR_OK;
}
Error
c_resize(void* self, void **in_out_ptr, usize_t size) {
    void *p = realloc(*in_out_ptr, size);
    if (p == NULL) {
        return _ctx.raise(ERROR_MEM_ALLOC);
    }

    *in_out_ptr = p;
    return ERROR_OK;
}
void
c_free(void *self, void **in_out_ptr) {
    free(*in_out_ptr);
    *in_out_ptr = NULL;
}

typedef struct {
    struct Allocator_Vtable_s _vtable;
} C_Allocator;

C_Allocator
c_allocator() {
    C_Allocator a;
    a._vtable = (struct Allocator_Vtable_s) {
            .alloc = c_alloc,
            .resize = c_resize,
            .free = c_free,
        };
    return a;
}

Error
default_raise(Error e) {
    return e;
}

C_Allocator _c_allocator;

void
ctx_init_default() {
    _c_allocator = c_allocator();
    _ctx.global_alloc = (Allocator*)&_c_allocator;
    _ctx.raise = default_raise;
}

#define SWAP(x, y) \
{                  \
    auto t = (y);  \
    (y) = (x);     \
    (x) = t;       \
}                  \

#define NEW(p) allocator_alloc(_ctx.global_alloc, sizeof(typeof(**p)), (void**)p)

#define FREE(p) allocator_free(_ctx.global_alloc, (void**)p)


// in C you can't process expressions, only build ones

#define THREAD_FN(type, name, body)              \
void *(name)(void *arg) {                        \
    (type) *thread_data = ((type)*)arg;          \
    body                                         \
}                                                \

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

typedef bool (*PredicateFn)(void*);
