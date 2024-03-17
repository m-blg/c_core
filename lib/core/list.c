#pragma once

#include "core/core.c"


#define FOR_IN_LIST(head, body)                                                 \
    for (auto _node_ = (head); _node_ != nullptr; _node_ = _node_->next) {      \
        body;                                                                   \
    }                                                                           \
    
/// TYPES: Node head, Node **out, bool (*pred)(Node*)
#define LIST_FIND(head, out, pred) {                                            \
    *out = nullptr;                                                             \
    for (auto _node_ = (head); _node_ != nullptr; _node_ = _node_->next) {      \
        if (pred) {                                                             \
            *out = _node_;                                                      \
            break;                                                              \
        }                                                                       \
    }                                                                           \
}                                                                               \

#define ListNode(T) ListNode_##T
#define list_node_proc(T, proc) list_node_##T##_##proc
#define ListIter(T) ListIter_##T

#define List(T) List
#define list_proc(T, proc) list_##T##_##proc
#define LIST_IMPL(T)                                               \
    LIST_IMPL_DECLS(T)                                            \
    LIST_IMPL_PROCS(list_##T,                                      \
                    T,                                             \
                    List(T),                                       \
                        head,                                      \
                        tail,                                      \
                        len,                                       \
                        allocator,                                 \
                    ListNode(T),                                   \
                        next                                       \
                    )                                              \
                                                                   
#define LIST_IMPL_DECLS(T)                                       \
struct ListNode(T) {                                               \
    struct ListNode(T) *next;                                      \
    T data;                                                        \
};                                                                 \
typedef struct ListNode(T) ListNode(T);                            \
                                                                   \
typedef struct {                                                   \
    ListNode(T) *head;                                             \
    ListNode(T) *tail;                                             \
    usize_t len;                                                   \
    Allocator *allocator;                                          \
} List(T);                                                         \

typedef struct {
    /*ListNode(T)*/void *cursor;
    usize_t len;
} ListIter(T);

                                                                   
                                                                   
#define LIST_IMPL_PROCS(__prefix,                                               \
                            T,                                                  \
                            Self,                                               \
                                _head_,                                         \
                                _tail_,                                         \
                                _len_,                                          \
                                _allocator_,                                    \
                            Node,                                               \
                                _next_,                                         \
                                _prev_)                                         \
                                                                                \
void                                                                            \
__prefix##_init(Self *self, Allocator *allocator) {                        \
    self->_head_ = nullptr;                                                     \
    self->_tail_ = nullptr;                                                     \
    self->_len_ = 0;                                                            \
    self->_allocator_ = allocator;                                              \
}                                                                               \
                                                                                \
Error                                                                           \
__prefix##_new_in(Allocator allocator[static 1],                                          \
                  Allocator self_allocator[static 1],                           \
                  Self **out_self)                                              \
{                                                                               \
    TRY(allocator_alloc(allocator, sizeof(Self), (void**)out_self));                   \
    __prefix##_init(*out_self, self_allocator);                                  \
    return ERROR_OK;                                                            \
}                                                                               \
                                                                                \
void                                                                            \
__prefix##_free(Self *self) {                                                   \
    for (auto node = self->head; node != nullptr; ) {                           \
        auto next = node->next;                                                 \
        allocator_free(self->allocator, (void**)&node);                         \
        node = next;                                                            \
    }                                                                           \
}                                                                              \
                                                                               \
/* CONTRACT: node should be allocated by self->allocator */                    \
void                                                                           \
__prefix##_push_node(Self *self, Node *node) {                                 \
    auto prev = self->_tail_;                                                  \
    self->_tail_ = node;                                                       \
                                                                               \
    prev->_next_ = node;                                                       \
    node->_next_ = nullptr;                                                    \
    node->_prev_ = prev;                                                       \
    self->len += 1;                                                            \
}                                                                              \
                                                                                
// /* CONTRACT: head should belong to list */                        
// Node *                                                            
// __prefix##_find(Node *head, bool (*pred)(Node*)) {                
//     auto node = head;                                             
//     while (node != nullptr) {                                     
//         if (pred(node)) {                                         
//             return node;                                          
//         }                                                         
//         node = node->_next_;                                      
//     }                                                             
//     return nullptr;                                               
// }                                                                 


struct ListVNS_Node {
    struct ListVNS_Node *next;
    struct ListVNS_Node *prev;
    uint8_t data[]; 
};
typedef struct ListVNS_Node ListVNS_Node;

/// @brief VNS - Variable Node Size
struct ListVNS {                  
    ListVNS_Node *head;           
    ListVNS_Node *tail;           
    usize_t len;                  
    Allocator allocator;         
};                                
typedef struct ListVNS ListVNS;   

struct ListIterVNS {
    ListVNS_Node *cursor;
    usize_t len;
};
typedef struct ListIterVNS ListIterVNS;

LIST_IMPL_PROCS(list_vns,                                     \
                uint8_t,                                      \
                ListVNS,                                      \
                    head,                                     \
                    tail,                                     \
                    len,                                      \
                    allocator,                                \
                ListVNS_Node,                                 \
                    next,                                     \
                    prev                                      \
                )                                             

/// @param[in] value: T* 
/// @param[in] fmt: StringFormatter* 
#define dbg_print_proc(T, value, fmt) T##_dbg_print(value, fmt)

typedef struct StringFormatter StringFormatter;
struct StringFormatter {
    usize_t pad_level;

    StreamWriter target;
};

typedef Error (*StreamWriter_WriteFn)(void *, usize_t, uint8_t[]);
typedef Error (*StreamWriter_FlushFn)(void *);

typedef struct StreamWriter StreamWriter;
struct StreamWriter {
    struct StreamWriter_VTable {
        StreamWriter_WriteFn write;
        StreamWriter_FlushFn flush;
    } _vtable;

    void *data;
};


INLINE
Error
stream_writer_write(StreamWriter self[static 1], usize_t data_size, uint8_t data[data_size]) {
    return self->_vtable.write(self->data, data_size, data);
}
INLINE
Error
stream_writer_flush(StreamWriter self[static 1]) {
    return self->_vtable.flush(self->data);
}

typedef struct StringLinearBuffer StringLinearBuffer;
struct StringLinearBuffer {
    Arena arena;
};

// Stream Writer Trait
Error
string_linear_buffer_write(StringLinearBuffer self[static 1], usize_t data_size, uint8_t data[data_size]) {
    return arena_allocator_alloc(self->arena, data_size, nullptr);
}
Error
string_linear_buffer_flush(StreamWriter self[static 1]) {
    unimplemented();
}

StreamWriter
string_linear_buffer_stream_writer(StringLinearBuffer self[static 1]) {
    return (StreamWriter) {
        ._vtable = (StreamWriter_VTable) {
            .write = string_linear_buffer_write,
            .flush = string_linear_buffer_flush,
        },
        .data = self,
    }
}
//

typedef struct StringRingBuffer StringRingBuffer;


typedef struct StringFormatter StringFormatter;
struct StringFormatter {
    usize_t pad_level;

    StreamWriter target;
};

void 
string_formatter_init(StringFormatter self[static 1]) {
    *self = (StringFormatter) {
        .pad_level = 0,
    };
}

Error
string_formatter_write(StringFormatter fmt[static 1], str_t s) {
    for_in_range(0, fmt.pad_level, {
        write(fmt->buffer, fmt->pad_string);
    });
    write(fmt->buffer, s);
}
Error
string_formatter_writeln(StringFormatter fmt[static 1], str_t s) {
    for_in_range(0, fmt.pad_level, {
        write(fmt->buffer, fmt->pad_string);
    });
    writeln(fmt->buffer, s);
}

#define DBG_PRINT 1
#ifdef DBG_PRINT
void
list_vns_node_dbg_print(ListVNS_Node self[static 1], StringFormatter fmt[static 1]) {
    usize_t_dbg_print((usize_t)&self->next, fmt);
    usize_t_dbg_print((usize_t)&self->prev, fmt);
    string_formatter_writeln(fmt, S("data[]:"));
}

void
list_vns_dbg_print(ListVNS self[static 1], StringFormatter fmt[static 1]) {
    string_formatter_writeln(fmt, S("ListVNS {"));
    string_formatter_pad_push(fmt);
        list_vns_node_dbg_print(self->head, fmt);
        list_vns_node_dbg_print(self->tail, fmt);
        usize_t_dbg_print(&self->len, fmt);
        allocator_dbg_print(self->allocator, fmt);
    string_formatter_pad_pop(fmt);
    string_formatter_write(fmt, S("}"));
}
#endif