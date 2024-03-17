#pragma once

#include "core/core.c"


// #define NULL (void *)0

typedef const char* cstr_t; 


typedef struct {
    union {
        uchar_t ascii;
        struct Rune1 {
            uchar_t c1;
        } r1;
        struct Rune2 {
            uchar_t c1, c2;
        } r2;
        struct Rune3 {
            uchar_t c1, c2, c3;
        } r3;
        struct Rune4 {
            uchar_t c1, c2, c3, c4;
        } r4;
    };
} CharUTF8;

typedef uint32_t rune_t;

Error 
get_next_rune(const CharUTF8 *ptr, const CharUTF8 **out_ptr) {
    if (ptr->ascii == '\0')
        return ERROR_STOP_ITERATOR;
    if (ptr->ascii < 0x80u) {
        *out_ptr = ptr + 1;
        return ERROR_OK;
    } else if (ptr->ascii < 0xe0u) {
        *out_ptr = ptr + 2;
        return ERROR_OK;
    } else if (ptr->ascii < 0xf0u) {
        *out_ptr = ptr + 3;
        return ERROR_OK;
    } else if (ptr->ascii < 0xf8u) {
        *out_ptr = ptr + 4;
        return ERROR_OK;
    }
    unreacheble();
}

rune_t
utf8_decode(const CharUTF8 *ptr) 
{
    if (ptr->r1.c1 < 0x80u) {
        return ptr->r1.c1;
    } else if (ptr->r2.c1 < 0xe0u) {
        return ((ptr->r2.c1 & 0x1f) << 6) | (ptr->r2.c2 & 0x3f);
    } else if (ptr->r3.c1 < 0xf0u) {
        return ((ptr->r3.c1 & 0x0f) << 12) | ((ptr->r3.c2 & 0x3f) << 6) | (ptr->r3.c3 & 0x3f);
    } else if (ptr->r4.c1 < 0xf8u) {
        return ((ptr->r4.c1 & 0x07) << 18) | ((ptr->r4.c2 & 0x3f) << 12) | ((ptr->r4.c3 & 0x3f) << 6) | (ptr->r4.c4 & 0x3f);
    }
    unreacheble();
}

// #define STRING_FREE(ptr)

/// Assumed to own memory
typedef struct {
    CharUTF8 *ptr;
    usize_t byte_cap;
    usize_t byte_len; // in bytes
    // usize_t rune_len; considered unneeded 
    Allocator *allocator;
} String;

/// Assumed to not own memory
typedef struct {
    CharUTF8 *ptr;
    usize_t byte_len; // in bytes
    // usize_t rune_len;
} str_t;

// #define BUFF_T(T)      
// typedef struct {       
//     T *ptr;            
//     usize_t byte_len;  
// } buff_t_##T;

// #define BUFF_IMPL(type_size, ptr, byte_len)    
    
// #define _STRING_ALLOC(type_size, count, out_ptr) 
//     _ctx.string_allocator.alloc((type_size), (count), (out_ptr))

// #define _STRING_FREE(ptr) _ctx.string_allocator.free((ptr))

/**
 *
 * \brief           string constructor, len = 0
 * \note            assumes alloc can fail, explicitly propagates error
 * \param[in]       self: string
 * \param[in]       cap: string capacity
 * \return          Error Code
 */
Error 
string_init(String *self, usize_t byte_cap, Allocator *allocator) {
    void *ptr;
    TRY(allocator_alloc(allocator, byte_cap, &ptr));

    *self = (String) {
        .ptr = ptr,
        .byte_cap = byte_cap,
        .allocator = allocator,
    };
    return ERROR_OK;
}

void string_free(String *self) {
    ASSERT(self && self->ptr);

    allocator_free(self->allocator, (void**)&self->ptr);
    memset(self, 0, sizeof(*self));
}

str_t 
str_from_c_str(const char *c_str) {
    const CharUTF8 *c = (CharUTF8*)c_str;
    const CharUTF8 *next = NULL;
    usize_t rune_len = 0;
    usize_t byte_len = 0;

    while (get_next_rune(c, &next) != ERROR_STOP_ITERATOR) {
        rune_len += 1;
        byte_len += (usize_t)(next - c);
        c = next;
    }

    return (str_t) {
        .ptr = (void *)c_str, 
        .byte_len = byte_len, 
        // .rune_len = rune_len,
    };
}


#define printlnf(fmt, ...) printf(fmt"\n", __VA_ARGS__)

#define S(c_str) (str_t) { .ptr  = (CharUTF8*)(c_str), .byte_len = sizeof(c_str)-1 }

#define len(x)                  \
    _Generic( (x),              \
        str_t: ((x).byte_len)   \
    )                           \

// Error string_from_str(String *self, str_t s) {
//   Error e;
//   if (e = string_init(self, s.len)) {
//     return e;
//   }
//   memcopy(self->ptr, s.ptr, s.len);
//   return ERROR_OK;
// }

// Error string_join_with(String *self, str_t sep, str_t s1, ...) {

//   usize_t len = sum;

//   void *ptr = _ctx.alloc(len);
//   if (ptr == NULL) {
//     return ERROR_MEM_ALLOC;
//   }
//   *self = (String){.ptr = ptr, .cap = len, .len = len};

//   for ()
//     memcopy();

//   return ERROR_OK;
// }

// str_t string_to_str(String *self) {
//   return (str_t) { .ptr = self->ptr, .len = self->len }
// }

// /**
//  *
//  * \brief           string view slice (inclusive)
//  * \note            inclusive
//  * \param[in]       self: string
//  * \param[in]       cap: string capacity
//  * \return          Error Code
//  */
// str_t str_slice(str_t self, isize_t from, isize_t to) {
//   ASSERT(from < self.len && to < self.len);
//   from = from % self.len;
//   to = to % self.len;
//   ASSERT(from <= to);

//   return (str_t) { .ptr = self.ptr + from, .len = to - from + 1 }
// }

// idea: intermediate value arena allocator

/*

Error
string_join(dbuff_t<str> str_list, str_t *out) {

    *out =
}

*/


#define for_in_range(i, from, to, body) for (auto i = (from); i < (to); i++) {body}

/**
 * @param T: Sized
*/
#define Slice(T) Slice_##T
#define slice_proc(T, proc) slice_##T##_##proc
#define SLICE_IMPL(T)                                               \
    SLICE_IMPL_STRUCT(T)                                            \
    SLICE_IMPL_PROCS(slice_##T, T, Slice(T), ptr, len)            \

#define SLICE_IMPL_STRUCT(T)                                       \
typedef struct {                                                   \
    T *ptr;                                                        \
    usize_t len;                                                   \
} Slice(T);                                                        \
                                                                                                                    
#define SLICE_IMPL_PROCS(__prefix, T, Self, _ptr_, _len_)                                                           \
                                                                                                                    \
Error                                                                                                               \
__prefix##_new_in(usize_t len, Allocator *a, Self *out_self) {                                                      \
    T *ptr;                                                                                                         \
    TRY(allocator_alloc_n(a, T, len, (void**)&ptr));                                                                     \
    *out_self = (Self) {                                                                                            \
        ._ptr_ = ptr,                                                                                                 \
        ._len_ = len,                                                                                                 \
    };                                                                                                              \
    return ERROR_OK;                                                                                                \
}


#define slice_len(slice) ((slice)->len)
#define slice_get(slice, i) ((slice)->ptr[(i)])
#define slice_size(self) ((self)->len * sizeof(*(self)->ptr))
#define slice_end(self) ((self)->ptr + (self)->len)                   

#define Option(T) Option_##T

#define OPTION_IMPL(T) \
typedef struct {       \
    bool is_some;      \
    T data;            \
} Option(T);           \
                       
#define Some(x) (Option(typeof(x))) { .is_some = true, .data = (x) }
#define None { 0 }

#define DArr(T) DArr_##T
#define DARR_IMPL(T)                                                                                                \
typedef struct {                                                                                                    \
    T *ptr;                                                                                                         \
    usize_t cap;                                                                                                    \
    usize_t len;                                                                                                    \
    Allocator allocator;                                                                                           \
} DArr(T);                                                                                                          \

/// @brief allowes for type annotation for variable element size type
#define DArrVES(T) DArrVES

/// @brief dynamic array with variable element size
/// @param el_size should be equal to sizeof(T), such that
///     you can get arr[n+1] = (uint8_t *)arr[n] + sizeof(T),
///     and arr[n] has alignment requirement of T, then arr[n+1] also has it
typedef struct DArrVES DArrVES;        
struct DArrVES {        
    void *ptr;             
    // Slice(void) data;
    usize_t el_size;
    usize_t cap;        
    usize_t len;        
    Allocator allocator;
};              


typedef DArrVES* darr_t;        
#define darr_t(T) darr_t

INLINE
void
darr_init(DArrVES self[static 1], usize_t el_size, Allocator allocator) {
    *self = (DArrVES) {
        .ptr = nullptr,
        .el_size = el_size,
        .allocator = allocator,
    };
}

/// @brief 
/// @param self 
/// @param el_size 
/// @param allocator 
/// @return AllocatorError 
AllocatorError 
darr_new_cap_in(usize_t el_size, usize_t cap, Allocator allocator[static 1], darr_t out_self[static 1]) {
    // TODO: test that alignment trick works
    usize_t size_alignment = sizeof(*darr_t) + (sizeof(*darr_t) % DEFAULT_ALIGNMENT);
    TRY(allocator_alloc(allocator, size_alignment + cap, DEFAULT_ALIGNMENT, (void **)out_self));
    **out_self = (DArrVES) {
        .ptr = (uint8_t *)*out_self + size_alignment,
        .el_size = el_size,
        .cap = cap,
        .allocator = *allocator,
    };
    return ERROR_OK;
}

void 
darr_free(darr_t self[static 1]) {
    auto alloc = self->allocator; // move out allocator
    allocator_free(&alloc, (void **)self);
}

#define darr_get_unchecked(T, self, index) ((T *)(self)->ptr + (index))
#define darr_get(T, self, index, item) {
    assert(0 <= index && index < (self)->len, "Index out of bounds");
    *(item) = ((T *)(self)->ptr + (index));
}
#define darr_rest(self) ((self)->cap - (self)->len)

#define DARR_DEFAULT_INIT_CAP 16

// self->cap can overflow
AllocatorError
darr_push(darr_t self[static 1], void *item) {
    // arrow op double deref used
    if (darr_rest(self) < self->el_size) {
        // reallocate
        usize_t new_cap = MAX(self->cap * 2, DARR_DEFAULT_INIT_CAP);
        darr_t new_self;
        TRY(darr_new_cap_in(self->el_size, new_cap, self->allocator, &new_self));
        new_self->len = self->len;
        // usize_t size_alignment = sizeof(*darr_t) + (sizeof(*darr_t) % DEFAULT_ALIGNMENT);
        // TRY(allocator_alloc(allocator, size_alignment + new_cap * self->el_size, DEFAULT_ALIGNMENT, (void **)&new_self));
        // memcpy(new_self, *self, size_alignment + self->cap * self->el_size);
        // *new_self = (DArrVES) {
        //     .ptr
        // }
        memcpy(new_self->ptr, self->ptr, self->cap * self->el_size);
        darr_free(self);
        *self = new_self;
    }
    auto dst = (uint8_t *)self->ptr + self->len * self->el_size;
    memcpy(dst, item, self->el_size);
    self->len += 1;
    return ALLOCATOR_ERROR(OK);
}

#define CircularBuffer(T) CircularBuffer_##T
#define circular_buffer_proc(T, proc) circular_buffer_##T##_##proc
#define CIRCULAR_BUFFER_IMPL(T)                                                                                     \
typedef struct {                                                                                                    \
    Slice(T) data;                                                                                                  \
    T *b_cursor;                                                                                                    \
    T *e_cursor;                                                                                                    \
    Allocator *allocator;                                                                                           \
} CircularBuffer(T);                                                                                                \
                                                                                                                    \
Error                                                                                                               \
circular_buffer_proc(T, new_in)(usize_t cap, Allocator *a, CircularBuffer(T) *out_self) {                             \
    Slice(T) data;                                                                                                  \
    TRY(slice_proc(T, new_in)(cap, a, &data));                                                                       \
    *out_self = (CircularBuffer(T)) {                                                                               \
        .data = data,                                                                                               \
        .b_cursor = data.ptr,                                                                                       \
        .e_cursor = data.ptr,                                                                                       \
        .allocator = a,                                                                                             \
    };                                                                                                              \
    return ERROR_OK;                                                                                                \
}                                                                                                                   \
void                                                                                                                \
circular_buffer_proc(T, free)(CircularBuffer(T) *self) {                                                              \
    allocator_free(self->allocator, (void**)&self->data.ptr);                                                               \
    NULLIFY(*self);                                                                                                 \
}                                                                                                                   \
                                                                                                                    \
/*inline*/                                                                                                              \
T *                                                                                                                 \
circular_buffer_proc(T, last)(CircularBuffer(T) *self) {                                                              \
    return self->e_cursor;                                                                                         \
}                                                                                                                   \
                                                                                                                    \
/**                                                                                                                 \
 * @param[in, out] self                                                                                             \
 * @param[in] value                                                                                                 \
*/                                                                                                                  \
void                                                                                                                \
circular_buffer_proc(T, push)(CircularBuffer(T) *self, T *value) {                                                     \
    ASSERT(circular_buffer_cap(self) > 0);                                                                                          \
    self->e_cursor += 1;                                                                                              \
    if (self->e_cursor == _circular_buffer_end(self)) {                                                               \
        self->e_cursor = self->data.ptr;                                                                                   \
    }                                                                                                               \
    memcpy(self->e_cursor, value, sizeof(T));                                                                         \
}                                                                                                                   \

/**
 * @param x: Sized
*/
#define NULLIFY(x) memset(&(x), 0, sizeof(x))
                                                                                 
/**                                                                              
 * @param[in] self: CircularBuffer(T) *                                          
*/                                                                               
#define _circular_buffer_end(self) slice_end(&self->data)
#define circular_buffer_cap(self) slice_size(&self->data)   

Error                                                                                                               
str_copy_in(str_t self, Allocator *a, str_t *out) {
    TRY(allocator_alloc(a, self.byte_len, (void**)&out->ptr));
    memcpy(out->ptr, self.ptr, self.byte_len);
    out->byte_len = self.byte_len;
    return ERROR_OK;
}

Error
str_from_c_str_in(cstr_t s, Allocator *a, str_t *out_self) {
    usize_t s_len = strlen(s);
    TRY(allocator_alloc(a, s_len, (void**)&out_self->ptr));
    memcpy(out_self->ptr, s, s_len);
    out_self->byte_len = s_len;
    return ERROR_OK;
}
#define str_from_c_str_no_copy(s) (str_t) {.ptr = (auto*)s, .byte_len = strlen(s)}

// Error
// string_copy_in(String self, String *out, Allocator *a) {
//     TRY(allocator_alloc(a, self.len, &out->ptr));
//     memcpy(out->ptr, self.ptr, self.len);
//     return ERROR_OK;
// }
Error
string_new_with_cap_in(usize_t byte_cap, Allocator *a, String *out_self) {
    TRY(allocator_alloc(a, byte_cap, (void**)&out_self->ptr));
    out_self->byte_cap = byte_cap;
    out_self->byte_len = 0;
    return ERROR_OK;
}
Error
string_from_in(str_t s, Allocator *a, String *out_self) {
    TRY(string_new_with_cap_in(s.byte_len, a, out_self));
    memcpy(out_self->ptr, s.ptr, s.byte_len);
    out_self->byte_len = s.byte_len;
    return ERROR_OK;
}
