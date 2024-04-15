#include "core/impl_guards.h"

#ifdef CORE_IMPL
#undef CORE_IMPL

#include "core/core.h"
#define CORE_IMPL
#include "core/core.h"

#define CORE_ARRAY_IMPL

#endif // CORE_IMPL

#ifdef CORE_ARRAY_IMPL
#define CORE_ARRAY_SLICE_IMPL
#define CORE_ARRAY_DARR_IMPL
#define CORE_ARRAY_RING_BUFF_IMPL
#endif // CORE_ARRAY_IMPL


#if CORE_HS_GUARD(CORE_ARRAY, SLICE)
#define CORE_ARRAY_H
#define CORE_ARRAY_SLICE_H

#include "core/core.h"


// "core/fmt.h"
enum_decl(FmtError);
struct_decl(StringFormatter);
typedef FmtError (FmtFn)(void *, StringFormatter *);
//

struct_def(SliceVES, { 
    void *ptr;             
    usize_t el_size;
    usize_t el_align;
    usize_t len;        

#ifdef CONTAINER_FMT
    FmtFn *el_fmt;
    FmtFn *el_dbg_fmt;
#endif
})

typedef SliceVES slice_t;
#define slice_T(T) slice_t

INLINE
usize_t
slice_size(slice_t *self) {
    return self->len * self->el_size;
}
INLINE
usize_t
slice_len(slice_t *self) {
    return self->len;
}

INLINE
void *
slice_get_unchecked(slice_t *self, usize_t index) {
    return (void *)((u8_t *)self->ptr + index * self->el_size);
}
#define slice_get_unchecked_T(T, self, index) ((T *)slice_get_unchecked(self, index))
INLINE
void *
slice_get(slice_t *self, usize_t index) {
    ASSERTM(0 <= index && index < self->len, "Index out of bounds");
    return slice_get_unchecked(self, index);
}
#define slice_get_T(T, self, index) ((T *)slice_get(self, index))

INLINE
void *
slice_get_i_unchecked(slice_t *self, isize_t index) {
    isize_t len = (isize_t)self->len;
    index = (len + index) % len;
    return (void *)((u8_t *)self->ptr + index * self->el_size);
}
#define slice_get_i_unchecked_T(T, self, index) ((T *)slice_get_i_unchecked(self, index))
INLINE
void *
slice_get_i(slice_t *self, isize_t index) {
    isize_t len = (isize_t)self->len;
    ASSERTM(-len <= index && index < len, "Index out of bounds");
    return slice_get_i_unchecked(self, index);
}
#define slice_get_iT(T, self, index) ((T *)slice_get_i(self, index))

INLINE
void *
slice_end(slice_t *self) {
    return slice_get_unchecked(self, self->len);
}

AllocatorError
slice_new_in(usize_t el_size, usize_t alignment, 
             usize_t len, Allocator *alloc, 
             slice_t *out_self);
#define slice_new_in_T(T, len, alloc, out_self) \
    slice_new_in(sizeof(T), alignof(T), len, alloc, out_self)

void
slice_free(slice_t *self, Allocator *alloc);
/// @param out_slice should be preallocated
void
slice_copy_data(slice_t *self, slice_t *out_slice);


#ifdef DBG_PRINT
FmtError
slice_dbg_fmt(slice_t *self, StringFormatter *fmt);
#endif // DBG_PRINT


#define c_arr_lit(x, ...) (typeof(x)[]) {(x), __VA_ARGS__}
#define c_arr_lit_len(arr) (sizeof(arr) / sizeof(typeof(0[arr])))

#define slice_lit(x, ...) ((slice_t) {                                      \
    .ptr = &c_arr_lit((x), __VA_ARGS__),                                     \
    .el_size = sizeof(x),                                       \
    .el_align = alignof(x),                                     \
    .len = c_arr_lit_len(c_arr_lit((x), __VA_ARGS__)),                                        \
})                                      \

#undef CORE_ARRAY_H
#endif // CORE_ARRAY_SLICE_H








#if CORE_IS_GUARD(CORE_ARRAY, SLICE)
#define CORE_ARRAY_SLICE_I

AllocatorError
slice_new_in(usize_t el_size, usize_t alignment, 
             usize_t len, Allocator *alloc, 
             slice_t *out_self) 
{ 
    void *ptr;                                                    
    TRY(allocator_alloc(alloc, len * el_size, alignment, &ptr));           
    *out_self = (slice_t) {                                       
        .ptr = ptr,                                          
        .el_size = el_size,
        .el_align = alignment,
        .len = len,                                          
    };                                                         
    return ALLOCATOR_ERROR(OK);                                           
}

void
slice_free(slice_t *self, Allocator *alloc) {
    allocator_free(alloc, (void**)&self->ptr);                                                               
    NULLIFY(*self);                                                                                                 
}
void
slice_clone_in(slice_t *self, Allocator *alloc, slice_t *out_slice) {
    unimplemented(); // TODO
}

/// @param out_slice should be preallocated
void
slice_copy_data(slice_t *self, slice_t *out_slice) {
    ASSERT(out_slice->len >= self->len);
    memcpy(out_slice->ptr, self->ptr, slice_size(self));
}

// void
// slice_i32_dbg_print(slice_t *self) {
//     void *ptr;             
//     usize_t el_size;
//     usize_t el_align;
//     usize_t len;        
//     #define PAD "    "
//     printf("slice {" "\n");
//         printf(PAD "len: %ld" "\n", self->len);
//         printf(PAD "data: [", self->len);
//         for_in_range(i, 0, darr_len(self), {
//             printf("%d", *darr_get_T(i32_t, self, i));
//         })
//         printf("]" "\n");
//     printf("}" "\n");
//     #undef PAD
// }

#ifdef DBG_PRINT


FmtError
slice_dbg_fmt(slice_t *self, StringFormatter *fmt) {
    auto fo = (Formattable) {
        ._vtable = (Formattable_VTable) {
            .fmt = self->el_dbg_fmt,
        },
    };
    TRY(string_formatter_write(fmt, S("[")));
    for_in_range(i, 0, self->len-1, {
        // TODO figure out a way to dispatch on type here (static or dynamic)
        fo.data = slice_get(self, i);
        TRY(formattable_fmt(&fo, fmt));
        TRY(string_formatter_write(fmt, S(", ")));
    })
    fo.data = slice_get_i(self, -1);
    TRY(formattable_fmt(&fo, fmt));
    TRY(string_formatter_write(fmt, S("]")));

    return FMT_ERROR(OK);
}

#endif

#endif // CORE_ARRAY_SLICE_I

#if CORE_HS_GUARD(CORE_ARRAY, DARR)
#define CORE_ARRAY_H
#define CORE_ARRAY_DARR_H

/// @brief dynamic array with variable element size
/// @param el_size should be equal to sizeof(T), such that
///     you can get arr[n+1] = (uint8_t *)arr[n] + sizeof(T),
///     and arr[n] has alignment requirement of T, then arr[n+1] also has it
struct_def(DArrVES, {        
    slice_t data;
    usize_t len;        
    Allocator allocator;
})

/// @brief allowes for type annotation for variable element size type
#define DArrVES_T(T) DArrVES

// INLINE
// void
// darr_ves_init(DArrVES *self, usize_t el_size, Allocator allocator) {
//     *self = (DArrVES) {
//         .ptr = nullptr,
//         .el_size = el_size,
//         .allocator = allocator,
//     };
// }

#define darr_ves_cap(self) ((self)->data.len)


typedef DArrVES * darr_t;        
#define DArrVES_T(T) DArrVES
#define darr_T(T) darr_t

#define darr_cap(self) ((self)->data.len)
#define darr_len(self) ((self)->len)
// #define darr_get_T(T, self, index) slice_get_T(T, &(self)->data, index)
// #define darr_get_iT(T, self, index) slice_get_iT(T, &(self)->data, index)
#define darr_rest(self) (darr_cap(self) - (self)->len)

#define DARR_DEFAULT_INIT_CAP 16

INLINE
void *
darr_get_unchecked(darr_t self, usize_t index) {
    return slice_get_unchecked(&self->data, index);
}
#define darr_get_unchecked_T(T, self, index) ((T *)darr_get_unchecked(self, index))
INLINE
void *
darr_get(darr_t self, usize_t index) {
    ASSERTM(0 <= index && index < self->len, "Index out of bounds");
    return darr_get_unchecked(self, index);
}
#define darr_get_T(T, self, index) ((T *)darr_get(self, index))

INLINE
void *
darr_get_i_unchecked(darr_t self, isize_t index) {
    isize_t len = (isize_t)self->len;
    index = (len + index) % len;
    return slice_get_unchecked(&self->data, index);
}
#define darr_get_i_unchecked_T(T, self, index) ((T *)darr_get_i_unchecked(self, index))
INLINE
void *
darr_get_i(darr_t self, isize_t index) {
    isize_t len = (isize_t)self->len;
    ASSERTM(-len <= index && index < len, "Index out of bounds");
    return darr_get_i_unchecked(self, index);
}
#define darr_get_iT(T, self, index) ((T *)darr_get_i(self, index))

#define darr_new_cap_in_T(T, cap, allocator, out_self)\
    darr_new_cap_in(sizeof(T), alignof(T), cap, allocator, out_self)


#undef CORE_ARRAY_H
#endif // CORE_ARRAY_DARR_H

#if CORE_IS_GUARD(CORE_ARRAY, DARR)
#define CORE_ARRAY_DARR_I

AllocatorError 
darr_new_cap_in(usize_t el_size, usize_t alignment, usize_t cap, Allocator *allocator, darr_t *out_self) {
    // TODO: test that alignment trick works (probably redo)
    
    // // DArrVes
    // // ^(1)   ^(2)
    // // (1), (2) are aligned with alignof(DArrVES), sizeof(DArrVES) guarantees that
    // // 
    // usize_t size_alignment = (alignment > alignof(DArrVES)) ? sizeof(DArrVES) + alignment - alignof(DArrVES) : sizeof(DArrVES);
    // // TRY(allocator_alloc(allocator, size_alignment + cap, alignment, (void **)out_self));
    // // breaks if alignment < alignof(DArrVES), so (DArrVES *) can be misaligned
    // TRY(allocator_alloc(allocator, size_alignment + cap * el_size   , alignof(DArrVES), (void **)out_self));

    void *data;
    TRY(alloc_two(sizeof(DArrVES), alignof(DArrVES), 
                  cap * el_size, alignment,
                  allocator,
                  (void **)out_self, (void **)&data));

    **out_self = (DArrVES) {
        .data = (slice_t) {
            .ptr = data,
            .el_size = el_size,
            .el_align = alignment,
            .len = cap,
        },
        .len = 0,
        .allocator = *allocator,
    };
    return ALLOCATOR_ERROR(OK);
}

void 
darr_free(darr_t *self) {
    auto alloc = (*self)->allocator; // move out allocator
    allocator_free(&alloc, (void **)self);
}

// #define darr_get_unchecked(T, self, index) ((T *)(self)->ptr + (index))
// #define darr_get(T, self, index, item) {
//     ASSERTM(0 <= index && index < (self)->len, "Index out of bounds");
//     *(item) = ((T *)(self)->ptr + (index));
// }

// self->cap can overflow
AllocatorError
darr_push(darr_t *self, void *item) {
    // arrow op double deref used (Not working btw)
    auto _self = *self;
    if (darr_rest(_self) == 0) {
        // reallocate
        usize_t new_cap = MAX(darr_cap(_self) * 2, DARR_DEFAULT_INIT_CAP);
        darr_t new_self;
        TRY(darr_new_cap_in(_self->data.el_size, _self->data.el_align, new_cap, &_self->allocator, &new_self));
        // usize_t size_alignment = sizeof(*darr_t) + (sizeof(*darr_t) % DEFAULT_ALIGNMENT);
        // TRY(allocator_alloc(allocator, size_alignment + new_cap * self->el_size, DEFAULT_ALIGNMENT, (void **)&new_self));
        // memcpy(new_self, *self, size_alignment + self->cap * self->el_size);
        // *new_self = (DArrVES) {
        //     .ptr
        // }
        slice_copy_data(&_self->data, &new_self->data);
        new_self->len = _self->len;
        // memcpy(new_self->ptr, _self->ptr, darr_cap(_self) * _self->data.el_size);
        auto p = *self;
        darr_free(&p);
        *self = new_self;
        _self = *self;
    }
    // auto dst = (u8_t *)_self->data.ptr + _self->len * _self->data.el_size;
    memcpy(slice_get_unchecked(&_self->data, _self->len), item, _self->data.el_size);
    _self->len += 1;
    return ALLOCATOR_ERROR(OK);
}

AllocatorError
darr_from_slice_in(slice_t *slice, Allocator *alloc, darr_t *self) {
    TRY(darr_new_cap_in(slice->el_size, slice->el_align, slice_len(slice), alloc, self));

    memcpy((*self)->data.ptr, slice->ptr, slice->len * slice->el_size);
    (*self)->len = slice_len(slice);
    
    return ALLOCATOR_ERROR(OK);
}

#define darr_lit(self, ...) {\
    auto sl = slice_lit(__VA_ARGS__);\
    TRY(darr_from_slice_in(&sl, &g_ctx.global_alloc, self));\
}\

#ifdef DBG_PRINT
void
darr_i32_dbg_print(darr_t self) {
    #define PAD "    "
    printf("darr_t {" "\n");
        printf(PAD "cap: %ld" "\n", darr_cap(self));
        printf(PAD "len: %ld" "\n", self->len);
        printf(PAD "allocator: %ld" "\n", (uintptr_t)self->allocator.data);
        printf(PAD "data: [");
        for_in_range(i, 0, darr_len(self) - 1, {
            printf("%d, ", *darr_get_T(i32_t, self, i));
        })
        printf("%d]\n", *darr_get_iT(i32_t, self, -1));
    printf("}" "\n");
    #undef PAD
}
#endif // DBG_PRINT

#endif // CORE_ARRAY_DARR

#if CORE_HS_GUARD(CORE_ARRAY, RING_BUFF)
#define CORE_ARRAY_H
#define CORE_ARRAY_RING_BUFF_H

struct_def(RingBuffVES, {                                                                                                    
    slice_t data;                                                                                                  
    void *b_cursor;                                                                                                    
    void *e_cursor;                                                                                                    
    usize_t len;
    Allocator allocator;                                                                                           
})

typedef RingBuffVES *ring_buff_t;
#define RingBuffVES_T(T) RingBuffVES
#define ring_buff_T(T) ring_buff_t

/// @param[in] self: CircularBuffer(T) *                                         
#define ring_buff_end(self) slice_end(&self->data)
#define ring_buff_cap(self) (self->data.len)
#define ring_buff_len(self) (self->len)

INLINE
void *                                                                                                                 
ring_buff_last(ring_buff_t self) {                                                              
    return self->e_cursor;                                                                                         
}                                                                                                                   

#undef CORE_ARRAY_H
#endif // CORE_ARRAY_RING_BUFF_H


#if CORE_IS_GUARD(CORE_ARRAY, RING_BUFF)
#define CORE_ARRAY_RING_BUFF_I

AllocatorError                                                                                                               
ring_buff_new_in(usize_t el_size, usize_t alignment, usize_t cap, Allocator *alloc, ring_buff_t *out_self) {                             
    void *data;
    TRY(alloc_two(sizeof(RingBuffVES), alignof(RingBuffVES), 
                  cap * el_size, alignment,
                  alloc,
                  (void **)out_self, (void **)&data));

    **out_self = (RingBuffVES) {
        .data = (slice_t) {
            .ptr = data,
            .el_size = el_size,
            .el_align = alignment,
            .len = cap,
        },
        .b_cursor = data,                                                                                       
        .e_cursor = data,                                                                                       
        .len = 0,
        .allocator = *alloc,                                                                                             
    };
    return ALLOCATOR_ERROR(OK);
}
void 
ring_buffer_free(ring_buff_t *self) {
    auto alloc = (*self)->allocator; // move out allocator
    allocator_free(&alloc, (void **)self);
}
                                                                                                                    
                                                                                                                    
/// @param[in, out] self                                                                                             
/// @param[in] value                                                                                                 
void                                                                                                                
ring_buff_push(ring_buff_t self, void *value) {                                                     
    // ASSERT(ring_buff_cap(self) > 0 && self->e_cursor == self->b_cursor);                                                                                          
    ASSERT(ring_buff_cap(self) != ring_buff_len(self)); // e_cursor can not overrun b_cursor
    auto index = (uintptr_t)self->e_cursor - (uintptr_t)self->data.ptr;
    self->e_cursor = (u8_t *)self->data.ptr + (index + 1) % self->len;                                                                                              
    // if (self->e_cursor == ring_buff_end(self)) {                                                               
    //     self->e_cursor = self->data.ptr;                                                                                   
    // }                                                                                                               
    memcpy(self->e_cursor, value, self->data.el_size);                                                                         
    self->len += 1;
}                                                                                                                   

#endif // CORE_ARRAY_RING_BUFF_I
