#pragma once

#include "core/core.c"
#include "core/iter.c"


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

IteratorError 
get_next_rune(const CharUTF8 *ptr, const CharUTF8 **out_ptr) {
    if (ptr->ascii == '\0')
        return ITERATOR_ERROR(STOP_ITERATOR);
    if (ptr->ascii < 0x80u) {
        *out_ptr = ptr + 1;
        return ITERATOR_ERROR(OK);
    } else if (ptr->ascii < 0xe0u) {
        *out_ptr = ptr + 2;
        return ITERATOR_ERROR(OK);
    } else if (ptr->ascii < 0xf0u) {
        *out_ptr = ptr + 3;
        return ITERATOR_ERROR(OK);
    } else if (ptr->ascii < 0xf8u) {
        *out_ptr = ptr + 4;
        return ITERATOR_ERROR(OK);
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
AllocatorError 
string_init(String *self, usize_t byte_cap, Allocator *allocator) {
    void *ptr;
    TRY(allocator_alloc(allocator, byte_cap, 1, &ptr));

    *self = (String) {
        .ptr = ptr,
        .byte_cap = byte_cap,
        .allocator = allocator,
    };
    return ALLOCATOR_ERROR(OK);
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

    while (get_next_rune(c, &next) != ITERATOR_ERROR(STOP_ITERATOR)) {
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






AllocatorError                                                                                                               
str_copy_in(str_t self, Allocator *a, str_t *out) {
    TRY(allocator_alloc(a, self.byte_len, sizeof(char), (void**)&out->ptr));
    memcpy(out->ptr, self.ptr, self.byte_len);
    out->byte_len = self.byte_len;
    return ALLOCATOR_ERROR(OK);
}

AllocatorError
str_from_c_str_in(cstr_t s, Allocator *a, str_t *out_self) {
    usize_t s_len = strlen(s);
    TRY(allocator_alloc(a, s_len, sizeof(char), (void**)&out_self->ptr));
    memcpy(out_self->ptr, s, s_len);
    out_self->byte_len = s_len;
    return ALLOCATOR_ERROR(OK);
}
#define str_from_c_str_no_copy(s) (str_t) {.ptr = (auto*)s, .byte_len = strlen(s)}

// Error
// string_copy_in(String self, String *out, Allocator *a) {
//     TRY(allocator_alloc(a, self.len, &out->ptr));
//     memcpy(out->ptr, self.ptr, self.len);
//     return ALLOCATOR_ERROR(OK);
// }
AllocatorError
string_new_with_cap_in(usize_t byte_cap, Allocator *a, String *out_self) {
    TRY(allocator_alloc(a, byte_cap, sizeof(char), (void**)&out_self->ptr));
    out_self->byte_cap = byte_cap;
    out_self->byte_len = 0;
    return ALLOCATOR_ERROR(OK);
}
AllocatorError
string_from_in(str_t s, Allocator *a, String *out_self) {
    TRY(string_new_with_cap_in(s.byte_len, a, out_self));
    memcpy(out_self->ptr, s.ptr, s.byte_len);
    out_self->byte_len = s.byte_len;
    return ALLOCATOR_ERROR(OK);
}
