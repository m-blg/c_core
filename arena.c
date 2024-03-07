#pragma once

#include "core/core.c"
#include "core/list.c"

struct ArenaChunk {
    struct ArenaChunk *next;
    uint8_t *cursor;
    usize_t data_size;
    uint8_t data[];
};
typedef struct ArenaChunk ArenaChunk;

/// chunk: ArenaChunk*
#define ARENA_CHUNK_REST_CAP(chunk) \
    ((chunk)->data_size - ((chunk)->cursor - (uint8_t*)(chunk)))

/// chunk: ArenaChunk*
/// returns ptr to the byte **after** the last byte of data
#define ARENA_CHUNK_END(chunk) \
    ((chunk)->data + (chunk)->data_size)

void
arena_chunk_reset(ArenaChunk *self) {
    self->cursor = self->data;
}
bool
arena_chunk_contains(ArenaChunk *self, uint8_t *ptr) {
    return self->data <= ptr && ptr < ARENA_CHUNK_END(self);
}

typedef struct {
    struct Allocator_Vtable_s _vtable;
    usize_t chunk_size;
    usize_t chunk_count;
    ArenaChunk *head;
    ArenaChunk *tail;
    Allocator *allocator;
} Arena;

LIST_IMPL(arena, Arena, ArenaChunk, chunk_count)
LIST_IMPL_PROCS(arena_chunk_list_##T,                                      \
                void,                                             \
                Arena,                                       \
                    head,                                      \
                    tail,                                      \
                    len,                                       \
                    allocator,                                 \
                ArenaChunk,                                   \
                    next                                       \
                )                                              \
// typedef struct {
//     AllocatorAllocFn alloc;
//     AllocatorResizeFn resize;
//     AllocatorFreeFn free;

//     Arena *arena;
// } ArenaAllocator;
// NOTE: instead of List object 
// create List protocol for generating implementation for this type


Error                        
arena_allocator_alloc(void *, usize_t , void **);
Error
arena_allocator_resize(void* , void **, usize_t );
void
arena_allocator_free(void *, void **);

#define ARENA_DEFAULT_CHUNK_SIZE 1024

// TODO: test this
Error
arena_init(
    Arena *self,
    usize_t chunk_size,
    Allocator *allocator)
{
    self->_vtable = (struct Allocator_Vtable_s) {
        .alloc = arena_allocator_alloc,
        .resize = arena_allocator_resize,
        .free = arena_allocator_free,
    };
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
    return ERROR_OK;
}

void
arena_reset(Arena *self) {
    // ArenaChunk *_node_;
    FOR_IN_LIST(self->head, {
        arena_chunk_reset(_node_);
    })
}

void
arena_free(Arena *self) {
    for (auto node = self->head; node != nullptr; ) {
        auto next = node->next;
        allocator_free(self->allocator, (void**)&node);
        node = next;
    }
}

Allocator * 
arena_allocator(Arena *self) {
    return (Allocator*)(self);
}

#define ARENA_CHUNK_SIZE(data_size) ((data_size) + sizeof(ArenaChunk))


Error                        
arena_allocator_alloc(void *self, usize_t data_size, void **out_ptr) 
{
    Arena *_self = (Arena*)self;
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
Error
arena_allocator_resize(void* self, void **in_out_ptr, usize_t data_size) {
    Arena *_self = (Arena*)self;

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
arena_allocator_free(void *self, void **in_out_ptr) {
    *in_out_ptr = nullptr;
}