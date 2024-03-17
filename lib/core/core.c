#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef DBG_PRINT
#define DBG_PRINT 1
#endif

typedef uint64_t usize_t;
typedef uint64_t isize_t;
typedef uint8_t uchar_t;
// typedef uchar_t bool;

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT alignof(max_align_t)
#endif

typedef enum AllocatorError AllocatorError;
enum AllocatorError {
    ALLOCATOR_ERROR_OK,
    ALLOCATOR_ERROR_MEM_ALLOC,
    ALLOCATOR_ERROR_COUNT
};

#define ALLOCATOR_ERROR(ERR) ((AllocatorError)ALLOCATOR_ERROR_##ERR)

typedef enum IteratorError IteratorError;
enum IteratorError {
    ITERATOR_ERROR_OK,
    ITERATOR_ERROR_STOP_ITERATOR,
    ITERATOR_ERROR_COUNT
};

typedef enum ErrorKind ErrorKind;
typedef struct Error Error;
struct Error {
    union {
        AllocatorError allocator_error;
        IteratorError iterator_error;
        isize_t value;
    };
    enum ErrorKind {
        ERROR_KIND_ALLOCATOR_ERROR,
        ERROR_KIND_ITERATOR_ERROR,
    } kind;
};


#define INLINE static inline __attribute((always_inline))

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

#define error_cast(e)                                                                              \
    _Generic((e),                                                                                  \
        AllocatorError: (Error) { .allocator_error = (e), .kind = ERROR_KIND_ALLOCATOR_ERROR },    \
        IteratorError: (Error) { .iterator_error = (e), .kind = ERROR_KIND_ITERATOR_ERROR }       \
        )                                                                                          \
        

#define RAISE(expr) {       \
        auto e = (expr);    \
        g_ctx.raise(error_cast(e));     \
        return e;           \
    }                       \


#define TRY(expr) {                         \
    auto e = (expr);                        \
    if (*(isize_t *)&e != 0) {              \
        return e;                           \
    }                                       \
  }                                         \

#define OR_RAISE(expr) {                         \
    auto e = (expr);                        \
    if (*(isize_t *)&e != 0) {              \
        return g_ctx.raise(error_cast(e));  \
    }                                       \
  }                                         \

void print_stack_trace() {} // TODO(mbgl)
// constexpr int foo() {return 3;}

#define ASSERT(expr)                         \
    if (!(expr)) {                           \
        panic();                             \
    }

#define ASSERT_OK(expr)                      \
    auto e = (expr);                         \
    if (*(isize_t *)&e != 0) {               \
        panic();                             \
    }                                        \

// design decisions:
//  ergonomics first, then 'speed'

// null ptr error encoding
// typedef void *(*AllocatorAllocFn)(Allocator*, usize_t, usize_t);
// typedef bool (*AllocatorResizeFn)(Allocator*, void *, usize_t, usize_t);
// typedef void (*AllocatorFreeFn)(Allocator*, void *);

// interface

#define CSlice(T, n) typeof(T[n])

typedef struct Allocator Allocator;
typedef struct Allocator_Vtable Allocator_Vtable;
struct Allocator {
    struct Allocator_Vtable {
        AllocatorError (*alloc)(void *, usize_t, usize_t, void **);
        AllocatorError (*resize)(void *, usize_t, usize_t, void **);
        void (*free)(void *, void **);
    } _vtable;

    void *data;
};

// opaque type instead of Allocator *
typedef AllocatorError (*AllocatorAllocAlignFn)(void *, usize_t, usize_t, void **);
typedef AllocatorError (*AllocatorResizeAlignFn)(void *, usize_t, usize_t, void **);
typedef void (*AllocatorFreeFn)(void *, void **);

__thread struct {
    Allocator *global_alloc;
    // TODO
    void (*raise)(Error);
} g_ctx;

#define NonNullPtr [static 1]

// Can not do allocations without alignment, so it is required
AllocatorError
allocator_alloc(Allocator* self, usize_t size, usize_t alignment, void **out_ptr) {
    return self->_vtable.alloc(self->data, size, alignment, out_ptr);
}
AllocatorError
allocator_resize(Allocator* self, usize_t size, usize_t alignment, void **in_out_ptr) {
    return self->_vtable.resize(self->data, size, alignment, in_out_ptr);
}
/// @param[in, out] ptr: NonNull
void 
allocator_free(Allocator* self, void **ptr) {
    self->_vtable.free(self->data, ptr);
}

AllocatorError
allocator_alloc_z(Allocator* self, usize_t size, usize_t alignment, void **out_ptr) {
    TRY(self->_vtable.alloc(self, size, alignment, out_ptr));
    memset((void*)*out_ptr, 0, size);
    return ALLOCATOR_ERROR(OK);
}

#define allocator_alloc_n(self, T, count, out_ptr) \
    allocator_alloc(self, sizeof(T) * count, out_ptr)

#define allocator_alloc_zn(self, T, count, out_ptr) \
    allocator_alloc_z(self, sizeof(T) * count, out_ptr)


// TODO(mblg): invoke gdb here
void panic() { 
    print_stack_trace(); 
    exit(1); 
}


AllocatorError
c_alloc(void *self, usize_t size, usize_t alignment, void **out_ptr) {
    // malloc return pointers with alignof(max_align_t), so alignment adjustments are not required
    void *p = malloc(size);
    if (p == nullptr) {
        RAISE(ALLOCATOR_ERROR(MEM_ALLOC));
    }

    *out_ptr = p;
    return ALLOCATOR_ERROR(OK);
}

AllocatorError
c_resize(void *self, usize_t size, usize_t alignment, void **in_out_ptr) {
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
default_raise(Error e) { }

Allocator g_c_allocator;

void
ctx_init_default() {
    g_c_allocator = c_allocator();
    g_ctx.global_alloc = (Allocator*)&g_c_allocator;
    g_ctx.raise = default_raise;
}

#define SWAP(x, y) \
{                  \
    auto t = (y);  \
    (y) = (x);     \
    (x) = t;       \
}                  \

/// @param[in] allocator: Allocator*
/// @param[out] p: T**
#define NEW_IN(allocator, p) allocator_alloc((allocator), sizeof(typeof(**(p))), alignof(typeof(**(p))), (void **)(p))
#define NEW(p) NEW_IN(g_ctx.global_alloc, p)

/// @param[in] allocator: Allocator*
/// @param[out] p: T**
#define FREE_IN(allocator, p) allocator_free((allocator), (void **)(p))
#define FREE(p) FREE_IN(g_ctx.global_alloc, p)


// in C you can't process expressions, only build ones

#define THREAD_FN(type, name, body)              \
void *(name)(void *arg) {                        \
    (type) *thread_data = ((type)*)arg;          \
    body                                         \
}                                                \

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

typedef bool (*PredicateFn)(void*);
