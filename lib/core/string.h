#ifdef CORE_IMPL
#undef CORE_IMPL

#include "core/core.h"
#define CORE_IMPL
#include "core/core.h"

#define CORE_STRING_IMPL
#endif // CORE_IMPL

#ifndef CORE_STRING_H
#define CORE_STRING_H

#include "core/core.h"
// #include "core/iter.h"


enum_def(UTF8_Error,
    UTF8_ERROR_OK,
    UTF8_ERROR_EMPTY_STRING,
    UTF8_ERROR_INCOMPLETE_RUNE,
    UTF8_ERROR_INVALID_RUNE,
)
#define UTF8_ERROR(ERR) ((UTF8_Error)UTF8_ERROR_##ERR)


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
get_next_rune(const CharUTF8 *ptr, const CharUTF8 **out_ptr);
rune_t
utf8_decode(const CharUTF8 *ptr);

/// Assumed to own memory
struct_def(String, {
    uchar_t *ptr;
    usize_t byte_cap;
    usize_t byte_len; // in bytes
    // usize_t rune_len; considered unneeded 
    Allocator allocator;
})

// struct_def(String, {
//     darr_T(uchar_t) data;
// })

/// Assumed to not own memory
struct_def(str_t, {
    uchar_t *ptr;
    usize_t byte_len; // in bytes
    // usize_t rune_len;
})

#define str_len(self) ((self).byte_len)
#define str_from_ptr_len(_ptr, _len) ((str_t) {.ptr = (uchar_t *)(_ptr), .byte_len = (_len)})

#define string_to_str(self) ((str_t) {.ptr = (self)->ptr, .byte_len = (self)->byte_len })

#define S(c_str) ((str_t) { .ptr  = (uchar_t *)(c_str), .byte_len = sizeof(c_str)-1 })

#define len(x)                  \
    _Generic( (x),              \
        str_t: ((x).byte_len)   \
    )                           \


AllocatorError 
string_init(String *self, usize_t byte_cap, Allocator *allocator);
INLINE
void 
string_reset(String *self);

AllocatorError
string_new_cap_in(usize_t byte_cap, Allocator *a, String *out_self);

AllocatorError
string_append_str(String *self, str_t str);
AllocatorError
string_prepend_str(String *self, str_t str);
AllocatorError
string_reserve_cap(String *self, usize_t reserve_cap);

UTF8_Error
str_next_rune(str_t self, rune_t *out_rune, str_t *out_self);

/// @brief return the length of the string in runes
/// @note linear complexity
/// @param self 
/// @return number of runes in the string
UTF8_Error
str_rune_len(str_t self, usize_t *out_len);

#ifdef DBG_PRINT

// "core/fmt.h"
struct_decl(StringFormatter)
enum_decl(FmtError)
//

// dbg_fmt_proc_decl(str, str_t, self, fmt)
FmtError
str_dbg_fmt(const str_t *self, StringFormatter *fmt, void *);

#endif // DBG_PRINT

// fmt_proc_decl(str, str_t, self, fmt)
FmtError
str_fmt(const str_t *self, StringFormatter *fmt, void *);

#endif // CORE_STRING_H 







#if defined(CORE_STRING_IMPL) && !defined(CORE_STRING_I)
#define CORE_STRING_I

UTF8_Error
str_rune_len(str_t self, usize_t *out_len) {
    register usize_t len = 0; 
    rune_t r;
    UTF8_Error e;
    while (true) {
        e = str_next_rune(self, &r, &self);
        if (e != UTF8_ERROR(OK)) {
            if (e == UTF8_ERROR(EMPTY_STRING)) {
                *out_len = len;
                return UTF8_ERROR(OK);
            }
            return e;
        }
        len += 1;
    }

    unreacheble();
}

UTF8_Error
str_next_rune(str_t self, rune_t *out_rune, str_t *out_self) {
    if (str_len(self) < 1) {
        return UTF8_ERROR(EMPTY_STRING);
    }

    auto ptr = (CharUTF8 *)self.ptr;
    // Decode
    u8_t d = 0;
    if (ptr->r1.c1 < 0x80u) {
        d = 1;
        *out_rune = ptr->r1.c1;
    } else if (ptr->r2.c1 < 0xe0u) {
        d = 2;
        if (str_len(self) < d) {
            return UTF8_ERROR(INCOMPLETE_RUNE);
        }
        *out_rune = ((ptr->r2.c1 & 0x1f) << 6) | (ptr->r2.c2 & 0x3f);
    } else if (ptr->r3.c1 < 0xf0u) {
        d = 3;
        if (str_len(self) < d) {
            return UTF8_ERROR(INCOMPLETE_RUNE);
        }
        *out_rune = ((ptr->r3.c1 & 0x0f) << 12) | ((ptr->r3.c2 & 0x3f) << 6) | (ptr->r3.c3 & 0x3f);
    } else if (ptr->r4.c1 < 0xf8u) {
        d = 4;
        if (str_len(self) < d) {
            return UTF8_ERROR(INCOMPLETE_RUNE);
        }
        *out_rune = ((ptr->r4.c1 & 0x07) << 18) | ((ptr->r4.c2 & 0x3f) << 12) | ((ptr->r4.c3 & 0x3f) << 6) | (ptr->r4.c4 & 0x3f);
    } else {
        return UTF8_ERROR(INVALID_RUNE);
    }

    *out_self = str_from_ptr_len(self.ptr + d, str_len(self) - d);
    return UTF8_ERROR(OK);
}

AllocatorError
runes_to_string(slice_T(rune_t) runes[non_null], String *out_string) {
    String s;
    TRY(string_new_cap_in(sizeof(CharUTF8) * slice_len(runes), &g_ctx.global_alloc, &s));
    for_in_range(i, 0, slice_len(runes), {
        // string_push();
    })

    return ALLOCATOR_ERROR(OK);
}

INLINE
uchar_t *
str_get_byte(str_t self, usize_t index) {
    return self.ptr + index;
}

INLINE
isize_t
wrap_index(isize_t index, usize_t len) {
    return (len + index) % len;
} 

str_t
str_byte_slice(str_t self, usize_t li, usize_t hi) {
    ASSERT(0 <= li && li <= hi && hi <= str_len(self));
    return (str_t) {
        .ptr = self.ptr + li,
        .byte_len = hi - li,
    };
}

bool
str_eq(str_t str1, str_t str2) {
    if (str_len(str1) != str_len(str2)) {
        return false;
    }

    for_in_range(i, 0, str_len(str1), {
        if (*str_get_byte(str1, i) != *str_get_byte(str2, i)) {
            return false;
        }
    })

    return true;
}

// TODO: test it
bool
str_is_prefix(str_t prefix, str_t str) {
    if (str_len(prefix) > str_len(str)) {
        return false;
    }
    return str_eq(prefix, str_byte_slice(str, 0, str_len(prefix)));
}

// IteratorError 
// get_next_rune(const char *ptr, const CharUTF8 **out_ptr) {
//     if (ptr->ascii == '\0')
//         return ITERATOR_ERROR(STOP_ITERATOR);
//     if (ptr->ascii < 0x80u) {
//         *out_ptr = ptr + 1;
//         return ITERATOR_ERROR(OK);
//     } else if (ptr->ascii < 0xe0u) {
//         *out_ptr = ptr + 2;
//         return ITERATOR_ERROR(OK);
//     } else if (ptr->ascii < 0xf0u) {
//         *out_ptr = ptr + 3;
//         return ITERATOR_ERROR(OK);
//     } else if (ptr->ascii < 0xf8u) {
//         *out_ptr = ptr + 4;
//         return ITERATOR_ERROR(OK);
//     }
//     unreacheble();
// }

// rune_t
// utf8_decode(const CharUTF8 *ptr) 
// {
//     if (ptr->r1.c1 < 0x80u) {
//         return ptr->r1.c1;
//     } else if (ptr->r2.c1 < 0xe0u) {
//         return ((ptr->r2.c1 & 0x1f) << 6) | (ptr->r2.c2 & 0x3f);
//     } else if (ptr->r3.c1 < 0xf0u) {
//         return ((ptr->r3.c1 & 0x0f) << 12) | ((ptr->r3.c2 & 0x3f) << 6) | (ptr->r3.c3 & 0x3f);
//     } else if (ptr->r4.c1 < 0xf8u) {
//         return ((ptr->r4.c1 & 0x07) << 18) | ((ptr->r4.c2 & 0x3f) << 12) | ((ptr->r4.c3 & 0x3f) << 6) | (ptr->r4.c4 & 0x3f);
//     }
//     unreacheble();
// }

// #define STRING_FREE(ptr)

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
    TRY(allocator_alloc(allocator, byte_cap, sizeof(uchar_t) * 16, &ptr));

    *self = (String) {
        .ptr = ptr,
        .byte_cap = byte_cap,
        .byte_len = 0,
        .allocator = *allocator,
    };
    return ALLOCATOR_ERROR(OK);
}

void 
string_free(String *self) {
    ASSERT(self && self->ptr);

    allocator_free(&self->allocator, (void**)&self->ptr);
    memset(self, 0, sizeof(*self));
}

INLINE
void 
string_reset(String *self) {
    self->byte_len = 0;
}

#define string_rest_cap(self) (self->byte_cap - self->byte_len)

AllocatorError
string_append_str(String *self, str_t str) {
    if (string_rest_cap(self) < str_len(str)) {
        TRY(string_reserve_cap(self, MAX(self->byte_cap * 2, self->byte_cap + str_len(str))));
    }

    memcpy(self->ptr + self->byte_len, str.ptr, str.byte_len);
    self->byte_len += str.byte_len;
    return ALLOCATOR_ERROR(OK);
}
// AllocatorError
// string_append_str_fmt(String *self, str_t fmt_str, ...) {
//     if (string_rest_cap(self) < str_len(str)) {
//         TRY(string_reserve_cap(self, MAX(self->byte_cap * 2, self->byte_cap + str_len(str))));
//     }

//     memcpy(self->ptr + self->byte_len, str.ptr, str.byte_len);
//     self->byte_len += str.byte_len;
//     return ALLOCATOR_ERROR(OK);
// }
AllocatorError
string_prepend_str(String *self, str_t str) {
    if (string_rest_cap(self) < str_len(str)) {
        TRY(string_reserve_cap(self, MAX(self->byte_cap * 2, self->byte_cap + str_len(str))));
    }

    memmove(self->ptr + str.byte_len, self->ptr, self->byte_len);
    memcpy(self->ptr, str.ptr, str.byte_len);
    self->byte_len += str.byte_len;
    return ALLOCATOR_ERROR(OK);
}
AllocatorError
string_reserve_cap(String *self, usize_t reserve_cap) {
    if (string_rest_cap(self) < reserve_cap) {
        TRY(allocator_resize(&self->allocator, 
            MAX(self->byte_cap * 2, self->byte_cap + reserve_cap), 
            alignof(uchar_t), (void **)&self->ptr));
    }
    return ALLOCATOR_ERROR(OK);
}

// str_t 
// str_from_c_str(const char *c_str) {
//     const CharUTF8 *c = (CharUTF8*)c_str;
//     const CharUTF8 *next = NULL;
//     usize_t rune_len = 0;
//     usize_t byte_len = 0;

//     while (get_next_rune(c, &next) != ITERATOR_ERROR(STOP_ITERATOR)) {
//         rune_len += 1;
//         byte_len += (usize_t)(next - c);
//         c = next;
//     }

//     return (str_t) {
//         .ptr = (void *)c_str, 
//         .byte_len = byte_len, 
//         // .rune_len = rune_len,
//     };
// }


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
string_new_cap_in(usize_t byte_cap, Allocator *alloc, String *out_self) {
    void *ptr;
    TRY(allocator_alloc(alloc, byte_cap, sizeof(uchar_t) * 16, &ptr));

    *out_self = (String) {
        .ptr = ptr,
        .byte_cap = byte_cap,
        .byte_len = 0,
        .allocator = *alloc,
    };
    return ALLOCATOR_ERROR(OK);
}
AllocatorError
string_from_in(str_t s, Allocator *alloc, String *out_self) {
    void *ptr;
    TRY(allocator_alloc(alloc, s.byte_len, alignof(uchar_t), &ptr));

    *out_self = (String) {
        .ptr = ptr,
        .byte_cap = s.byte_len,
        .byte_len = s.byte_len,
        .allocator = *alloc,
    };
    memcpy(out_self->ptr, s.ptr, s.byte_len);
    return ALLOCATOR_ERROR(OK);
}

#include "core/fmt/fmt.h"

FmtError
str_fmt(const str_t *self, StringFormatter *fmt, void *_) {
    TRY(string_formatter_write(fmt, *self));
    return FMT_ERROR(OK);
}

#ifdef DBG_PRINT


FmtError
str_dbg_fmt(const str_t *self, StringFormatter *fmt, void *_) {
    TRY(string_formatter_write(fmt, S("\"")));
    TRY(string_formatter_write(fmt, *self));
    TRY(string_formatter_write(fmt, S("\"")));
    return FMT_ERROR(OK);
}

#endif // DBG_PRINT

#endif // CORE_STRING_IMPL