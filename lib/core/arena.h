#ifdef CORE_IMPL
#define CORE_ARENA_IMPL
#endif // CORE_IMPL

#ifndef CORE_ARENA_H
#define CORE_ARENA_H

#include "core/core.h"
#include "core/list.h"

struct ArenaChunk {
    struct ArenaChunk *next;
    uint8_t *cursor;
    usize_t data_size;
    uint8_t data[];
};
typedef struct ArenaChunk ArenaChunk;

/// @param[in] chunk: ArenaChunk*
#define ARENA_CHUNK_REST_CAP(chunk) \
    ((chunk)->data_size - ((chunk)->cursor - (uint8_t*)(chunk)))

/// @param[in] chunk: ArenaChunk*
/// @returns ptr to the byte **after** the last byte of data
#define ARENA_CHUNK_END(chunk) \
    ((chunk)->data + (chunk)->data_size)

void
arena_chunk_reset(ArenaChunk *self);
bool
arena_chunk_contains(ArenaChunk *self, uint8_t *ptr);

typedef struct Arena Arena;
struct Arena {
    usize_t chunk_size;
    list_T(ArenaChunk) chunks;
    Allocator allocator;
};

typedef struct ArenaAllocator_dyn ArenaAllocator_dyn;
struct ArenaAllocator_dyn {
    Allocator_Vtable _vtable;
    Arena data;
}

AllocatorError                        
arena_allocator_alloc(Arena[non_null], usize_t, usize_t, void **);
AllocatorError
arena_allocator_resize(Arena[non_null], usize_t, usize_t, void **);
void
arena_allocator_free(Arena[non_null], void **);

#define ARENA_DEFAULT_CHUNK_SIZE 1024


INLINE
usize_t
arena_chunk_count(static Arena *self) {
    return self->chunk.len;
}

void
arena_chunk_reset(ArenaChunk *self) {
    self->cursor = self->data;
}
bool
arena_chunk_contains(ArenaChunk *self, uint8_t *ptr) {
    return self->data <= ptr && ptr < ARENA_CHUNK_END(self);
}

#endif // CORE_ARENA_H

#if defined(CORE_ARENA_IMPL) && !defined(CORE_ARENA_I)
#define CORE_ARENA_I
// TODO: test this
AllocatorError
arena_init(
    Arena *self,
    usize_t chunk_size,
    Allocator *allocator)
{
    usize_t allocation_size = chunk_size + sizeof(ArenaChunk*);

    ArenaChunk *chunk;
    TRY(allocator_alloc(allocator, allocation_size, (void**)&chunk));

    memset(chunk, 0, allocation_size);

    *self = (Arena) {
        .chunk_size = chunk_size,
        .chunk_count = 1,
        .head = chunk,
        .tail = chunk,
        .allocator = allocator,
    };
    return ALLOCATOR_ERROR(OK);
}

void
arena_reset(Arena self[non_null]) {
    // ArenaChunk *_node_;
    FOR_IN_LIST(self->head, {
        arena_chunk_reset(_node_);
    })
}

void
arena_free(Arena self[non_null]) {
    for (auto node = self->head; node != nullptr; ) {
        auto next = node->next;
        allocator_free(self->allocator, (void**)&node);
        node = next;
    }
}

// AllocatorError                        
// _arena_allocator_alloc(Arena[non_null], usize_t , void **);
// AllocatorError
// _arena_allocator_resize(Arena[non_null] , void **, usize_t );
// void
// _arena_allocator_free(Arena[non_null], void **);

INLINE
Allocator
arena_allocator(Arena self[non_null]) {
    AllocatorError                        
    _arena_allocator_alloc(void *self, usize_t data_size, uint8_t *out_ptr[data_size]) {
        return arena_allocator_alloc((Arena *)self, data_size, out_ptr);
    }

    return (Allocator) {
        ._vtable = (Allocator_Vtable) {
            .alloc = _arena_allocator_alloc,
            .resize = _arena_allocator_resize,
            .free = _arena_allocator_free,
        },
        .data = self,
    };
}
#define ARENA_CHUNK_SIZE(data_size) ((data_size) + sizeof(ArenaChunk))


AllocatorError                        
arena_allocator_alloc(Arena self[non_null], usize_t data_size, uint8_t *out_ptr[data_size]) 
{
    // Arena *_self = (Arena*)self;
    // ArenaChunk *chunk = list_find_arena_chunk(_self->head, &chunk, ___arena_allocator_alloc_pred);
    ArenaChunk *chunk;
    LIST_FIND(_self->head, &chunk, ARENA_CHUNK_REST_CAP(_node_) >= data_size);
    if (chunk == nullptr) {
        // allocate new chunk to fit the data
        TRY(allocator_alloc(_self->allocator, ARENA_CHUNK_SIZE(MAX(data_size, _self->chunk_size)), (void**)&chunk));
        chunk->data_size = data_size;
        chunk->cursor = chunk->data;
        arena_list_push_node(_self, chunk);
    }
    // NOTE: at this point the chunk is valid
    // insert data
    *out_ptr = chunk->cursor;
    chunk->cursor += data_size;

    return ERROR_OK;
}
AllocatorError
arena_allocator_resize(Arena self[non_null], void **in_out_ptr, usize_t data_size) {
    // Arena *_self = (Arena*)self;

    ArenaChunk *chunk;
    LIST_FIND(_self->head, &chunk, arena_chunk_contains(_node_, *in_out_ptr));
    if (chunk == nullptr) {
        TRY(arena_allocator_alloc(self, data_size, in_out_ptr));
        return ERROR_OK;
    }

    // ASSUME: chunk is valid     

    return ERROR_OK;
}

/// doesn't deallocate memory
void
arena_allocator_free(Arena self[non_null], void **in_out_ptr) {
    *in_out_ptr = nullptr;
}

#endif // CORE_ARENA_IMPL