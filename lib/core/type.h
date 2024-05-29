
#ifndef CORE_TYPE_H
#define CORE_TYPE_H

#include "core/string.h"

struct_def(TypeInfo_VTable, {
    FmtFn *fmt;
    FmtFn *dbg_fmt;

    EqFn *eq;
    SetFn *set;
    HashFn *hash;
})
struct_def(TypeInfo, {
    usize_t size;
    usize_t align;
    str_t name;

    TypeInfo_VTable _vtable;
})

#define typeid_of(T) TYPE_ID_##T

#ifndef TYPE_LIST
#define TYPE_LIST \
    TYPE_LIST_ENTRY(int), \
    TYPE_LIST_ENTRY(usize_t), \
    TYPE_LIST_ENTRY(str_t), \
    TYPE_LIST_ENTRY(ArenaChunk), \
    TYPE_LIST_ENTRY(darr_t)

#endif // TYPE_LIST

typedef enum TypeId TypeId;
enum TypeId {
#define TYPE_LIST_ENTRY(T) typeid_of(T)
    TYPE_LIST
#undef TYPE_LIST_ENTRY
};

#endif // CORE_TYPE_H