#ifdef CORE_IMPL
#define CORE_ARRAY_IMPL
#endif // CORE_IMPL

#ifdef CORE_ARRAY_IMPL
#undef CORE_ARRAY_IMPL

#define CORE_ARRAY_SLICE_IMPL
#define CORE_ARRAY_DARR_IMPL
#define CORE_ARRAY_RING_BUFF_IMPL

#define CORE_CORE_IMPL
#include "core/core.h"
#endif // CORE_ARRAY_IMPL

// =======================
#ifndef CORE_ARRAY_SLICE_H
#define CORE_ARRAY_SLICE_H

#define CORE_NO_IMPL
#include "core/core.h"
#undef CORE_NO_IMPL
// #include "core/string.h"

struct_def(SliceVES, { 
    void *ptr;             
    usize_t el_size;
    usize_t el_align;
    usize_t len;        

#ifdef DBG_PRINT
    // FmtFn *el_fmt;
#endif
})

typedef SliceVES slice_t;
#define slice_T(T) slice_t

INLINE
usize_t
slice_size(slice_t self[static 1]) {
    return self->len * self->el_size;
}
INLINE
usize_t
slice_len(slice_t self[static 1]) {
    return self->len;
}

INLINE
void *
slice_get_unchecked(slice_t self[static 1], usize_t index) {
    return (void *)((u8_t *)self->ptr + index * self->el_size);
}
#define slice_get_unchecked_T(T, self, index) ((T *)slice_get_unchecked(self, index))
INLINE
void *
slice_get(slice_t self[static 1], usize_t index) {
    ASSERTM(0 <= index && index < self->len, "Index out of bounds");
    return slice_get_unchecked(self, index);
}
#define slice_get_T(T, self, index) ((T *)slice_get(self, index))

INLINE
void *
slice_get_i_unchecked(slice_t self[static 1], isize_t index) {
    isize_t len = (isize_t)self->len;
    index = (len + index) % len;
    return (void *)((u8_t *)self->ptr + index * self->el_size);
}
#define slice_get_i_unchecked_T(T, self, index) ((T *)slice_get_i_unchecked(self, index))
INLINE
void *
slice_get_i(slice_t self[static 1], isize_t index) {
    isize_t len = (isize_t)self->len;
    ASSERTM(-len <= index && index < len, "Index out of bounds");
    return slice_get_i_unchecked(self, index);
}
#define slice_get_iT(T, self, index) ((T *)slice_get_i(self, index))

INLINE
void *
slice_end(slice_t self[static 1]) {
    return slice_get_unchecked(self, self->len);
}

AllocatorError
slice_new_in(usize_t el_size, usize_t alignment, 
             usize_t len, Allocator alloc[static 1], 
             slice_t *out_self);
#define slice_new_in_T(T, len, alloc, out_self) \
    slice_new_in(sizeof(T), alignof(T), len, alloc, out_self)

void
slice_free(slice_t self[static 1], Allocator alloc[static 1]);
/// @param out_slice should be preallocated
void
slice_copy_data(slice_t *self, slice_t *out_slice);

#endif // CORE_ARRAY_SLICE_H


// =======================
#if CORE_IMPL_GUARD(CORE_ARRAY_SLICE)
#define CORE_ARRAY_SLICE_I

AllocatorError
slice_new_in(usize_t el_size, usize_t alignment, 
             usize_t len, Allocator alloc[static 1], 
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
slice_free(slice_t self[static 1], Allocator alloc[static 1]) {
    allocator_free(alloc, (void**)&self->ptr);                                                               
    NULLIFY(*self);                                                                                                 
}
void
slice_clone_in(slice_t self[static 1], Allocator alloc[static 1], slice_t out_slice[static 1]) {
    unimplemented(); // TODO
}

/// @param out_slice should be preallocated
void
slice_copy_data(slice_t *self, slice_t *out_slice) {
    ASSERT(out_slice->len >= self->len);
    memcpy(out_slice->ptr, self->ptr, slice_size(self));
}

// void
// slice_i32_dbg_print(slice_t self[static 1]) {
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

// #ifdef DBG_PRINT

// // FmtError
// // slice_dbg_fmt(self, fmt) {
// //     auto fo = (Formattable) {
// //         ._vtable = (Formattable_VTable) {
// //             .fmt = self->el_fmt,
// //         },
// //     };
// //     string_formatter_write(S("["));
// //     for_in_range(i, 0, self->len, {
// //         // TODO figure out a way to dispatch on type here (static or dynamic)
// //         fo.data = slice_get(self, i);
// //         formattable_fmt(&fo, fmt);
// //         string_formatter_write(S(", "));
// //     })
// //     string_formatter_write(S("["));
// // }

// #endif
#endif // CORE_ARRAY_SLICE_IMPL


// =======================
#ifndef CORE_ARRAY_DARR_H
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
// darr_ves_init(DArrVES self[static 1], usize_t el_size, Allocator allocator) {
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

#endif // CORE_ARRAY_DARR_H


// =======================
#if CORE_IMPL_GUARD(CORE_ARRAY_DARR)
#define CORE_ARRAY_DARR_I

AllocatorError 
darr_new_cap_in(usize_t el_size, usize_t alignment, usize_t cap, Allocator allocator[static 1], darr_t out_self[static 1]) {
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
darr_free(darr_t self[static 1]) {
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
darr_push(darr_t self[static 1], void *item) {
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

#endif // CORE_ARRAY_DARR_IMPL

// ring buffer

#ifndef CORE_ARRAY_RING_BUFF_H
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

#endif // CORE_ARRAY_RING_BUFF_H

// =======================
#if CORE_IMPL_GUARD(CORE_ARRAY_RING_BUFF)
#define CORE_ARRAY_RING_BUFF_I

AllocatorError                                                                                                               
ring_buff_new_in(usize_t el_size, usize_t alignment, usize_t cap, Allocator alloc[static 1], ring_buff_t *out_self) {                             
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
ring_buffer_free(ring_buff_t self[static 1]) {
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

#endif // CORE_ARRAY_RING_BUFF_IMPL

                                                                                 