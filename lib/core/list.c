#pragma once
#define CORE_LIST_IMPL

#include "core/core.c"
#include "core/fmt.c"


struct_def(ListVES_Node, {
    ListVES_Node *next;
    ListVES_Node *prev;

    alignas(MAX_ALIGNMENT) 
    u8_t data[]; 
})

/// @brief VES - Variable Element Size
struct_def(ListVES, {
    ListVES_Node *head;
    ListVES_Node *tail;
    usize_t el_size;
    usize_t len;
    Allocator allocator;
})

typedef ListVES * list_t
#define ListVES_T(T) ListVES
#define list_T(T) list_t

struct_def(ListIterVES, {
    ListVES_Node *cursor;
    usize_t len;
})

void
list_init(Self *self, Allocator *allocator) {
    self->_head_ = nullptr;
    self->_tail_ = nullptr;
    self->_len_ = 0;
    self->_allocator_ = allocator;
}
Error
list_new_in(Allocator allocator[static 1],
                  Allocator self_allocator[static 1],
                  Self **out_self)
{
    TRY(allocator_alloc(allocator, sizeof(Self), (void**)out_self));
    list_init(*out_self, self_allocator);
    return ERROR_OK;
}
void
list_free(Self *self) {
    for (auto node = self->head; node != nullptr; ) {
        auto next = node->next;
        allocator_free(self->allocator, (void**)&node);
        node = next;
    }
}
/* CONTRACT: node should be allocated by self->allocator */
void
list_push_node(Self *self, Node *node) {
    auto prev = self->_tail_;
    self->_tail_ = node;
    prev->_next_ = node;
    node->_next_ = nullptr;
    node->_prev_ = prev;
    self->len += 1;
}

// #define DBG_PRINT 1
// #ifdef DBG_PRINT

// #define PAR_NS(ns, T, proc) ns##_##T##_##proc
// #define NS(ns, proc) ns##_##proc
// // #define list_ves_dbg_fmt_proc(T) list_ves_##T##_dbg_fmt
// #define list_ves_dbg_fmt(T, self, fmt) PAR_NS(list_ves, T, dbg_fmt)(self, fmt)
// #define LIST_VNS_NODE_DBG_FMT_PROCS(T)

// void
// PAR_NS(list_ves_node, T, dbg_fmt)(ListVES_Node self[static 1], StringFormatter fmt[static 1]);
// void
// PAR_NS(list_ves, T, dbg_fmt)(ListVES self[static 1], StringFormatter fmt[static 1]);
// #ifdef CORE_LIST_IMPL
// void
// PAR_NS(list_ves_node, T, dbg_fmt)(ListVES_Node self[static 1], StringFormatter fmt[static 1]) {
//     usize_t_dbg_fmt((usize_t)&self->next, fmt);
//     usize_t_dbg_fmt((usize_t)&self->prev, fmt);
//     string_formatter_writeln(fmt, S("data[]:"));
//     dbg_fmt((T *)self->data, fmt);
// }

// void
// PAR_NS(list_ves, T, dbg_fmt)(ListVES self[static 1], StringFormatter fmt[static 1]) {
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


// #define STREAM_WRITER_STDOUT (StreamWriter) {\
//     .\
// }\

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