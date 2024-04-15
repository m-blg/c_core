
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
 * @param[in] self: CircularBuffer(T) *                                          
*/                                                                               
#define _circular_buffer_end(self) slice_end(&self->data)
#define circular_buffer_cap(self) slice_size(&self->data)   


// =========================

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
__prefix##_new_in(Allocator allocator[non_null],                                          \
                  Allocator self_allocator[non_null],                           \
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