#include "core/impl_guards.h"

#ifdef CORE_IMPL
#define CORE_LIST_IMPL
#endif // CORE_IMPL

#ifndef CORE_LIST_H
#define CORE_LIST_H

#include "core/core.h"
#include "core/fmt/fmt.h"


struct_def(List_dynT_Node, {
    List_dynT_Node *next;
    List_dynT_Node *prev;

    // want to support arbitrary alignment
    // alignas(MAX_ALIGNMENT) 
    // u8_t data[]; 
})

struct_def(List_dynT, {
    List_dynT_Node *head;
    List_dynT_Node *tail;
    usize_t len;
    usize_t node_data_size;
    usize_t node_data_align;
    TypeId node_data_tid;
    Allocator alloc;
})

typedef List_dynT * list_t;
#define list_T(T) list_t

struct_def(List_dynT_Iter, {
    List_dynT_Node *cursor;
    usize_t len;
    usize_t node_data_align;
})

AllocatorError
list_node_alloc(List_dynT *self, void *data, List_dynT_Node **out_node);
INLINE
void *
list_node_data(List_dynT_Node *node, usize_t data_align);


#define list_len(self) ((self)->len)

INLINE
void
list_init(List_dynT *self, TypeId el_tid, Allocator *alloc);
AllocatorError
list_new_in(Allocator alloc[non_null],
            TypeId el_tid,
            list_t *out_self);
void
list_free(list_t *self);
void
list_free_arena(list_t *self);

void
list_push_node(list_t self, List_dynT_Node *node);
AllocatorError
list_push(list_t self, void *data);

// #define for_in_list(item, list) for (auto item = (list)->head; item != nullptr; item = item.next)
#define for_in_list_T(T, item, list, body) for (auto _item_ = (list)->head; _item_ != nullptr; _item_ = _item_->next) { \
    T *item = list_node_data(_item_, list->node_data_align); \
    body \
} \

INLINE
List_dynT_Iter
list_iter(list_t self);

INLINE
void NLB(*)
list_iter_next(List_dynT_Iter *iter);

#endif // CORE_LIST_H

#if CORE_IMPL_GUARD(CORE_LIST)
#define CORE_LIST_I

AllocatorError
list_node_alloc(List_dynT *self, void *data, List_dynT_Node **out_node)  {
    void *_data;
    TRY(alloc_sequentially_two(sizeof(List_dynT_Node), alignof(List_dynT_Node), 
                  self->node_data_size, self->node_data_align,
                  &self->alloc,
                  (void **)out_node, (void **)&_data));
    
    memcpy(_data, data, self->node_data_size);

    return ALLOCATOR_ERROR(OK);
} 
AllocatorError
list_node_alloc_size_align(List_dynT *self, usize_t data_size, usize_t data_align, List_dynT_Node **out_node)  {
    void *_data;
    TRY(alloc_sequentially_two(sizeof(List_dynT_Node), alignof(List_dynT_Node), 
                  data_size, data_align,
                  &self->alloc,
                  (void **)out_node, (void **)&_data));
    
    return ALLOCATOR_ERROR(OK);
} 

INLINE
void *
list_node_data(List_dynT_Node *node, usize_t data_align)  {
    return align_forward(node + 1, data_align);
}

INLINE
void
list_init(List_dynT *self, TypeId el_tid, Allocator *alloc) {
    *self = (List_dynT) {
        .node_data_size = type_prop(el_tid, size),
        .node_data_align = type_prop(el_tid, align),
        .node_data_tid = el_tid,
        .alloc = *alloc,
    };
}
AllocatorError
list_new_in(Allocator alloc[non_null],
            TypeId el_tid,
            list_t *out_self)
{
    TRY(allocator_alloc(alloc, sizeof(List_dynT), alignof(List_dynT), (void**)out_self));
    list_init(*out_self, el_tid, alloc);
    return ALLOCATOR_ERROR(OK);
}
void
list_free(list_t *self) {
    auto alloc = (*self)->alloc;
    for (auto node = (*self)->head; node != nullptr; ) {
        auto next = node->next;
        allocator_free(&alloc, (void**)&node);
        node = next;
    }
    allocator_free(&alloc, (void**)self);
}
void
list_free_arena(list_t *self) {
    auto alloc = (*self)->alloc;
    allocator_free(&alloc, (void**)self);
}

/// CONTRACT: node should be allocated by self->allocator
/// @param[in, out] self
void
list_push_node(list_t self, List_dynT_Node *node) {
    if ((self)->head == nullptr) {
        (self)->head = node;
        (self)->tail = node;
        self->len += 1;
        return;
    }
    auto prev = self->tail;
    self->tail = node;
    prev->next = node;
    node->next = nullptr;
    node->prev = prev;
    self->len += 1;
}

/// @param[in, out] self
AllocatorError
list_push(list_t self, void *data) {
    List_dynT_Node *node;
    TRY(list_node_alloc(self, data, &node));
    list_push_node(self, node);
    return ALLOCATOR_ERROR(OK);
}

INLINE
List_dynT_Iter
list_iter(list_t self) {
    return (List_dynT_Iter) {
        .cursor = self->head,
        .len = self->len,
        .node_data_align = self->node_data_align,
    };
}

INLINE
void NLB(*)
list_iter_next(List_dynT_Iter *iter) {
    if (iter->cursor == nullptr) {
        return nullptr;
    }

    auto cur = iter->cursor;
    iter->cursor = iter->cursor->next;
    iter->len -= 1;
    return list_node_data(cur, iter->node_data_align);
}

#endif // CORE_LIST_IMPL

// #define DBG_PRINT 1
// #ifdef DBG_PRINT

// #define PAR_NS(ns, T, proc) ns##_##T##_##proc
// #define NS(ns, proc) ns##_##proc
// // #define list_ves_dbg_fmt_proc(T) list_ves_##T##_dbg_fmt
// #define list_ves_dbg_fmt(T, self, fmt) PAR_NS(list_ves, T, dbg_fmt)(self, fmt)
// #define LIST_VNS_NODE_DBG_FMT_PROCS(T)

// void
// PAR_NS(list_ves_node, T, dbg_fmt)(ListVES_Node self[non_null], StringFormatter fmt[non_null]);
// void
// PAR_NS(list_ves, T, dbg_fmt)(ListVES self[non_null], StringFormatter fmt[non_null]);
// #ifdef CORE_LIST_IMPL
// void
// PAR_NS(list_ves_node, T, dbg_fmt)(ListVES_Node self[non_null], StringFormatter fmt[non_null]) {
//     usize_t_dbg_fmt((usize_t)&self->next, fmt);
//     usize_t_dbg_fmt((usize_t)&self->prev, fmt);
//     string_formatter_writeln(fmt, S("data[]:"));
//     dbg_fmt((T *)self->data, fmt);
// }

// void
// PAR_NS(list_ves, T, dbg_fmt)(ListVES self[non_null], StringFormatter fmt[non_null]) {
//     string_formatter_writeln(fmt, S("ListVNS {"));
//     string_formatter_pad_push(fmt);
//         list_ves_node_dbg_fmt(&self->head, fmt);
//         list_ves_node_dbg_fmt(&self->tail, fmt);
//         usize_t_dbg_fmt(&self->len, fmt);
//         allocator_dbg_print(&self->allocator, fmt);
//     string_formatter_pad_pop(fmt);
//     string_formatter_write(fmt, S("}"));
// }

// #endif


// #define STREAM_WRITER_STDOUT (StreamWriter) {
//     .
// }

// #define STRING_FORMATTER_DEFAULT_DBG_PRINT_PAD_STRING "    "
// #define STRING_FORMATTER_DEFAULT_DBG_PRINT
// (StringFormatter) {
//     .pad_level = 0,
//     .pad_string = STRING_FORMATTER_DEFAULT_DBG_PRINT_PAD_STRING,
//     .target = STREAM_WRITER_STDOUT,
// }
// #define dbg_print(dbg_fmt, self) {
//     auto fmt = STRING_FORMATTER_DEFAULT_DBG_PRINT;
//     (dbg_fmt)((self), fmt);
// }

// #endif
