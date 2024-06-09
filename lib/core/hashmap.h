#include "core/impl_guards.h"


#ifndef CORE_HASHMAP_H
#define CORE_HASHMAP_H

#include "core/core.h"
#include "core/array.h"

// struct_def(HashMap_Entry, {

// })
// struct_def(HashMap, {
//     void *data;
//     usize_t cap;
//     usize_t count;

//     usize_t entry_size;
//     usize_t entry_align;
// })

// typedef u32_t (HashFn)(const u8_t *bytes, usize_t len);
typedef u64_t (HashFn)(void *key);
typedef bool (EqFn)(void *val1, void *val2);
typedef void (SetFn)(void *lval, void *rval);

struct_def(HashMap_Bucket, {
    void *key;
    void *value;
})

#define SliceVES_T(T) SliceVES

struct_def(HashMap, {
    SliceVES_T(HashMap_Bucket) buckets;
    SliceVES keys;
    SliceVES values;

    usize_t count;
    
    HashFn *key_hash;
    EqFn *key_eq;
    SetFn *key_set;

    SetFn *value_set;
    TypeId key_tid;
    TypeId value_tid;

    Allocator alloc;
})

typedef HashMap * hashmap_t;

// hashmap_T(str_t, any_t)
#define hashmap_T(key, val) hashmap_t

#define hashmap_len(self) ((self)->count)

#define hashmap_new_cap_in_T(KT, VT, cap, alloc, out_map) \
    hashmap_new_cap_in(typeid_of(KT), typeid_of(VT), cap, alloc, out_map)


AllocatorError
hashmap_new_cap_in(
    TypeId key_tid,
    TypeId value_tid,
    usize_t cap, Allocator *alloc, hashmap_t *out_map);
void
hashmap_free(hashmap_t *self);

INLINE
usize_t
hashmap_value_size(HashMap *self);

INLINE
usize_t
hashmap_key_size(HashMap *self);
INLINE
usize_t
hashmap_bucket_size(HashMap *self);
INLINE
usize_t
hashmap_cap(HashMap *self);
INLINE
usize_t
hashmap_rest_cap(HashMap *self);

HashMap_Bucket NLB(*)
hashmap_get_bucket(HashMap *self, void *key);

HashMap_Bucket *
hashmap_get_bucket_by_intern(HashMap *self, void *key_obj);

INLINE
void NLB(*)
hashmap_get(HashMap *self, void *key);
#define hashmap_get_T(T_Val, self, key) ((T_Val *)hashmap_get(self, key))

#define hashmap_interned_key(key) key
INLINE
void NLB(*)
hashmap_get_by_intern(HashMap *self, hashmap_interned_key(void) *key_obj);

AllocatorError
hashmap_grow(hashmap_t *self, usize_t fit_cap);

#define _hashmap_value_set(self, lval, rval) \
    (type_vt((self)->value_tid, set)(lval, rval))

AllocatorError
hashmap_add_key_val(hashmap_t *self, void *key, void *value, HashMap_Bucket *bucket);

AllocatorError
hashmap_set(hashmap_t *self, void *key, void *value);

#define slice_from_ptr_len_T(T, _ptr, _len) ((slice_t) {\
    .ptr = (_ptr),\
    .len = (_len),\
    .el_size = sizeof(T),\
    .el_align = alignof(T),\
})\


#define for_in_hashmap_key_val_T(KT, VT, map, key, val, body) \
for (usize_t __i = 0; __i < hashmap_len(map); __i += 1) { \
    KT *key = slice_get_T(KT, &map->keys, __i); \
    VT *val = slice_get_T(VT, &map->keys, __i); \
    \
    body \
}

#endif // CORE_HASHMAP_H




#if CORE_IMPL_GUARD(CORE_HASHMAP)
#define CORE_HASHMAP_I

INLINE
usize_t
hashmap_value_size(HashMap *self) {
    return self->values.el_size;
}

INLINE
usize_t
hashmap_key_size(HashMap *self) {
    return self->keys.el_size; 
}
INLINE
usize_t
hashmap_bucket_size(HashMap *self) {
    return self->buckets.el_size; 
}
INLINE
usize_t
hashmap_cap(HashMap *self) {
    return self->buckets.len; 
}
INLINE
usize_t
hashmap_rest_cap(HashMap *self) {
    return self->buckets.len - self->count; 
}

HashMap_Bucket NLB(*)
hashmap_get_bucket(HashMap *self, void *key) {
    u32_t hash = self->key_hash(key);
    usize_t ind = hash % slice_len(&self->buckets);
    auto buck = slice_get_T(HashMap_Bucket, &self->buckets, ind);
    if (buck->key == nullptr) {
        return buck;
    }

    for_in_range(i, 0, slice_len(&self->buckets)) {
        if (slice_get_T(HashMap_Bucket, &self->buckets, ind)->key == nullptr ||
           self->key_eq(slice_get_T(HashMap_Bucket, &self->buckets, ind)->key, key)) 
        {
            return slice_get_T(HashMap_Bucket, &self->buckets, ind);
        }
        ind = (ind + 1) % slice_len(&self->buckets);
    }

    return nullptr;
}

HashMap_Bucket *
hashmap_get_bucket_by_intern(HashMap *self, void *key_obj) {
    u32_t hash = self->key_hash(key_obj);
    usize_t ind = hash % slice_len(&self->buckets);
    auto buck = slice_get_T(HashMap_Bucket, &self->buckets, ind);
    if (buck->key == nullptr) {
        return buck;
    }

    for_in_range(i, 0, slice_len(&self->buckets)) {
        if (slice_get_T(HashMap_Bucket, &self->buckets, ind)->key == nullptr ||
           slice_get_T(HashMap_Bucket, &self->buckets, ind)->key == key_obj) 
        {
            return slice_get_T(HashMap_Bucket, &self->buckets, ind);
        }
        ind = (ind + 1) % slice_len(&self->buckets);
    }

    return nullptr;
}


INLINE
void NLB(*)
hashmap_get(HashMap *self, void *key) {
    auto bucket = hashmap_get_bucket(self, key);
    if (bucket == nullptr || bucket->key == nullptr) {
        return nullptr;
    }
    return bucket->value;
}


INLINE
void NLB(*)
hashmap_get_by_intern(HashMap *self, hashmap_interned_key(void) *key_obj) {
    auto bucket = hashmap_get_bucket_by_intern(self, key_obj);
    if (bucket == nullptr || bucket->key == nullptr) {
        return nullptr;
    }
    return bucket->value;
}

AllocatorError
hashmap_grow(hashmap_t *self, usize_t fit_cap) {
    if (hashmap_rest_cap(*self) > fit_cap) {
        return ALLOCATOR_ERROR(OK);
    }

    usize_t new_cap = MAX(hashmap_cap(*self) * 2, hashmap_cap(*self) + fit_cap);
    usize_t size_aligns[4][2] = {
        [0] = { sizeof(HashMap), alignof(HashMap) },
        [1] = { new_cap * sizeof(HashMap_Bucket), alignof(HashMap_Bucket) },
        [2] = { new_cap * (*self)->keys.el_size, (*self)->keys.el_align },
        [3] = { new_cap * (*self)->values.el_size, (*self)->values.el_align },
    };
    void *ptrs[4];
    TRY(alloc_sequentially_n(4, size_aligns, &(*self)->alloc, &ptrs));

    hashmap_t out_map = ptrs[0];
    *out_map = **self;
    out_map->buckets.ptr = ptrs[1];
    out_map->buckets.len = new_cap;
    out_map->keys.ptr = ptrs[2];
    out_map->keys.len = new_cap;
    out_map->values.ptr = ptrs[3];
    out_map->values.len = new_cap;

    // reinsert
    slice_copy_data(&(*self)->keys, &out_map->keys);
    slice_copy_data(&(*self)->values, &out_map->values);
    for_in_range(i, 0, slice_len(&(*self)->keys)) {
        auto key = slice_get(&out_map->keys, i);
        auto value = slice_get(&out_map->values, i);
        auto bucket = hashmap_get_bucket(out_map, key);
        ASSERT(bucket != nullptr);
        bucket->key = key;
        bucket->value = value;
    }

    SWAP(*self, out_map);
    hashmap_free(&out_map);

    return ALLOCATOR_ERROR(OK);
}

AllocatorError
hashmap_add_key_val(hashmap_t *self, void *key, void *value, HashMap_Bucket *bucket) {
    if ((*self)->count >= hashmap_cap(*self)-1) {
        TRY(hashmap_grow(self, 2));
        bucket = hashmap_get_bucket(*self, key);
        ASSERT(bucket != nullptr);
    }

    auto count = (*self)->count;
    bucket->key = slice_get_unchecked(&(*self)->keys, count);
    bucket->value = slice_get_unchecked(&(*self)->values, count);

    if ((*self)->key_set) {
        (*self)->key_set(bucket->key, key);
    } else {
        memcpy(bucket->key, key, (*self)->keys.el_size);
    }
    if ((*self)->value_set) {
        (*self)->value_set(bucket->value, value);
    } else {
        memcpy(bucket->value, value, (*self)->values.el_size);
    }

    (*self)->count += 1;

    return ALLOCATOR_ERROR(OK);
}

AllocatorError
hashmap_set(hashmap_t *self, void *key, void *value) {
    auto bucket = hashmap_get_bucket(*self, key);
    if (bucket == nullptr || bucket->key == nullptr) {
        TRY(hashmap_add_key_val(self, key, value, bucket));
        return ALLOCATOR_ERROR(OK);
    }
    if ((*self)->value_set) {
        (*self)->value_set(bucket->value, value);
    } else {
        memcpy(bucket->value, value, (*self)->values.el_size);
    }
    // _hashmap_value_set(*self, bucket->value, value);
    return ALLOCATOR_ERROR(OK);
}


AllocatorError
hashmap_new_cap_in(
    TypeId key_tid,
    TypeId value_tid,
    usize_t cap, Allocator *alloc, hashmap_t *out_map)
{
    usize_t size_aligns[4][2] = {
        [0] = { sizeof(HashMap), alignof(HashMap) },
        [1] = { cap * sizeof(HashMap_Bucket), alignof(HashMap_Bucket) },
        [2] = { cap * type_prop(key_tid, size), type_prop(key_tid, align) },
        [3] = { cap * type_prop(value_tid, size), type_prop(value_tid, align) },
    };
    void *ptrs[4];
    TRY(alloc_sequentially_n(4, size_aligns, alloc, &ptrs));

    *out_map = ptrs[0];
    **out_map = (HashMap) {
        .buckets = slice_from_ptr_len_T(HashMap_Bucket, ptrs[1], cap),
        .keys = (slice_t) {
            .ptr = ptrs[2],
            .len = cap,
            .el_size = type_prop(key_tid, size),
            .el_align = type_prop(key_tid, align),
        },
        .key_hash = type_vt(key_tid, hash),
        .key_eq = type_vt(key_tid, eq),
        .key_set = type_vt(key_tid, set),
        .values = (slice_t) {
            .ptr = ptrs[3],
            .len = cap,
            .el_size = type_prop(value_tid, size),
            .el_align = type_prop(value_tid, align),
        },
        .value_set = type_vt(value_tid, set),
        .key_tid = key_tid,
        .value_tid = value_tid,
        .count = 0,
        .alloc = *alloc,
    };
    return ALLOCATOR_ERROR(OK);
}

void
hashmap_free(hashmap_t *self) {
    auto alloc = (*self)->alloc; // move out allocator
    allocator_free(&alloc, (void **)self);
}

#endif // CORE_HASHMAP_IMPL