#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifndef DBG_PRINT
#define DBG_PRINT 1
#endif

typedef uint64_t usize_t;
typedef uint8_t uchar_t;
typedef int64_t isize_t;

typedef uint64_t u64_t;
typedef uint32_t u32_t;
typedef uint16_t u16_t;
typedef uint8_t  u8_t;
typedef int64_t i64_t;
typedef int32_t i32_t;
typedef int16_t i16_t;
typedef int8_t  i8_t;

#define MAX_ALIGNMENT alignof(max_align_t)

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
#define ITERATOR_ERROR(ERR) ((IteratorError)ITERATOR_ERROR_##ERR)

typedef enum ErrorKind ErrorKind;
typedef struct Error Error;
struct Error {
    union {
        AllocatorError allocator_error;
        IteratorError iterator_error;
        int value;
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

void print_stack_trace() {} // TODO(mbgl)
// TODO(mblg): invoke gdb here
void panic() { 
    print_stack_trace(); 
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
    if (*(int *)&e != 0) {              \
        return e;                           \
    }                                       \
  }                                         \

#define OR_RAISE(expr) {                         \
    auto e = (expr);                        \
    if (*(int *)&e != 0) {              \
        return g_ctx.raise(error_cast(e));  \
    }                                       \
  }                                         \


#define ASSERT(expr)                         \
    if (!(expr)) {                           \
        panic();                             \
    }

#define ASSERTM(expr, msg)                   \
    if (!(expr)) {                           \
        fprintf(stderr, "%*s", (int)(sizeof(msg)-1), msg);\
        panic();                             \
    }

#define ASSERT_OK(expr)                      \
    auto e = (expr);                         \
    if (*(isize_t *)&e != 0) {               \
        panic();                             \
    }                                        \

/// @param x: Sized
#define NULLIFY(x) memset(&(x), 0, sizeof(x))

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

/// @brief Allocates array of T of len *count*. 
///    Each element is aligned with the alignment requirement for T.
#define allocator_alloc_n(self, T, count, out_ptr) \
    allocator_alloc(self, sizeof(T) * count, alignof(T), out_ptr)

#define allocator_alloc_zn(self, T, count, out_ptr) \
    allocator_alloc_z(self, sizeof(T) * count, alignof(T), out_ptr)



/// @brief V
/// @param value 
/// @return 
/// @paragraph Proof
/// 1. if value is a power of 2:
/// bin(value) = 0{0,m}10{0,n}
/// bin(value-1) = 0{0,m}01{0,n}
/// bin(value & (value-1)) = 0{1,m+n+1} = bin(0)
///
/// 2. if value is Not a power of 2:
/// bin(value) = 0{0,m}1(0|1){0,k}10{0,n}
/// bin(value-1) = 0{0,m}1(0|1){0,k}01{0,n}
/// bin(value & (value-1)) = 0{0,m}1(0|1){0,k}0{1,n} != bin(0)
///
/// Thus by cases: (value & (value-1)) == 0 iff value is power of 2
INLINE
bool
is_power_of_two(usize_t value) {
    return (value & (value - 1)) == 0;
}

INLINE
void *
align_forward(void *ptr, usize_t alignment) {
    return (void *)((u8_t *)ptr + ((uintptr_t)ptr % alignment));
}

INLINE
void *
ptr_shift(void *ptr, isize_t offset) {
    return (void *)((u8_t *)ptr + offset);
}

/// size1, size2 - adhere to sizeof semantics
/// align1, align2 - adhere to alignof semantics
AllocatorError 
alloc_two(usize_t size1, usize_t align1, 
          usize_t size2, usize_t align2,
          Allocator allocator[static 1], 
          void **out1, void **out2) 
{
    void *p = nullptr;
    usize_t pad = (align2 > align1) ? align2 - align1 : 0;
    TRY(allocator_alloc(allocator, size1 + pad + size2, align1, &p));
    *out1 = p;
    *out2 = align_forward(ptr_shift(p, size1), align2);
    return ALLOCATOR_ERROR(OK);
}


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

#define for_in_range(i, from, to, body) for (auto i = (from); i < (to); i++) {body}



#define struct_def(name, fields) \
typedef struct name name;         \
struct name fields;               \

#define enum_def(name, ...) \
typedef enum name name;         \
enum name {__VA_ARGS__};               \


#define printlnf(fmt, ...) printf(fmt"\n", __VA_ARGS__)