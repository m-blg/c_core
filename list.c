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

#define List(T) List_##T
#define list_proc(T, proc) list_##T##_##proc
#define LIST_IMPL(T)                                               \
    LIST_IMPL_STRUCT(T)                                            \
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
                                                                   
#define LIST_IMPL_STRUCTS(T)                                       \
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
    ListNode(T) *cursor;
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
                                _next_)                                         \
                                                                                \
void                                                                            \
__prefix##_init(Self *self, Allocator *allocator) {                        \
    self->_head_ = nullptr;                                                     \
    self->_tail_ = nullptr;                                                     \
    self->_len_ = 0;                                                            \
    self->_allocator_ = allocator;                                              \
}                                                                               \
                                                                                \
Node *                                                                          
__prefix##_free(Self *self) {                                          
    for (auto _node = (head); _node != nullptr; _node = _node->next) {          
        allocator_free(self->allocator, _node);                                 
    }                                                                           
}                                                                               
                                                                                \
/* CONTRACT: node should be allocated by self->allocator */                     \
void                                                                            \
__prefix##_push_node(Self *self, Node *node) {                             \
    self->_tail_->_next_ = node;                                                \
    self->_tail_ = node;                                                        \
    node->_next_ = nullptr;                                                     \
}                                                                               \
                                                                                \
// /* CONTRACT: head should belong to list */                                      \
// Node *                                                                          \
// __prefix##_find(Node *head, bool (*pred)(Node*)) {                         \
//     auto node = head;                                                           \
//     while (node != nullptr) {                                                   \
//         if (pred(node)) {                                                       \
//             return node;                                                        \
//         }                                                                       \
//         node = node->_next_;                                                    \
//     }                                                                           \
//     return nullptr;                                                             \
// }                                                                               \


