#pragma once

#define iter_next(iter) _Generic((*iter),         \
    ListIter: list_iter_next(iter)        \
    )                                       \

#define for_in(iter, item, body)  {                  \
    auto _iter_ = (iter);                            \
    typeof(iter_next(_iter_)) item = iter_next(_iter_);       \
    for (; item != nullptr; item = iter_next(_iter_)) {   \
        body                                         \
    }                                                \
}

#define iter_find(iter, pred, out_item) {                                       \
    for_in(iter, node, {                                                        \
        if (pred(node)) {                                                       \
            out_item = node;                                                    \
        }                                                                       \
    })                                                                          \
    out_item = nullptr;                                                         \
}                                                                               \

#define iter_find(iter, pred, out_item) {                                       \
    for_in(iter, node, {                                                        \
        if (pred(node)) {                                                       \
            out_item = node;                                                    \
        }                                                                       \
    })                                                                          \
    out_item = nullptr;                                                         \
}                                                                               \

#define ITER_IMPL(__prefix, Iter, Item)                                                 \
Item *                                                                          \
__prefix##_find(Iter *iter, bool (*pred)(Item*)) {                              \
    for_in(iter, node, {                                                        \
        if (pred(node)) {                                                       \
            return node;                                                        \
        }                                                                       \
    })                                                                          \
    return nullptr;                                                             \
}                                                                               \


#define FILTER_ITER_IMPL(Iter, Item)          \
typedef struct {                   \
    Iter *iter;                                \
    bool (*pred)(Item *)                        \
} FilterIter(Iter);                                            \
                                              \
Item *                                        \
filter_next(Iter *self) {                     \
    auto item = next(self->iter);             \
    while (item != nullptr) {                 \
        if (self->pred(item)) {               \
            return item;                      \
        }                                     \
        item = next(self->iter);              \
    }                                         \
    return nullptr;                           \
}                                             \