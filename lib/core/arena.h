#include "core/impl_guards.h"

#ifdef CORE_IMPL
#define CORE_ARENA_IMPL
#endif // CORE_IMPL

#if CORE_HEADER_GUARD(CORE_ARENA)
#define CORE_ARENA_H

#include "core/core.h"
#include "core/list.h"
#include "core/iter.h"

struct_def(ArenaChunk, {
    u8_t *cursor;
    usize_t data_size;
    u8_t data[];
    // void *data;
})

INLINE
usize_t
arena_chunk_rest_cap(ArenaChunk *chunk);

/// @brief rest capacity of the chunk after aligning the cursor
INLINE
usize_t
arena_chunk_rest_cap_aligned(ArenaChunk *chunk, usize_t alignment);

/// @param[in] chunk: ArenaChunk*
/// @returns ptr to the byte **after** the last byte of data
INLINE
u8_t *
arena_chunk_end(ArenaChunk *chunk);

INLINE
void
arena_chunk_reset(ArenaChunk *self);
INLINE
bool
arena_chunk_contains(ArenaChunk *self, u8_t *ptr);

#define ArenaChunk_fmt nullptr
#define ArenaChunk_dbg_fmt nullptr
#define ArenaChunk_eq nullptr
#define ArenaChunk_set nullptr
#define ArenaChunk_hash nullptr



struct_def(Arena, {
    list_T(ArenaChunk) chunks;
    usize_t default_chunk_data_size;
})

// struct_def(ArenaAllocator_dyn, {
//     Allocator_Vtable _vtable;
//     Arena data;
// })


AllocatorError
arena_init(Arena *self, usize_t default_chunk_data_size, Allocator *alloc);
void
arena_reset(Arena self[non_null]);
void
arena_deinit(Arena self[non_null]);
INLINE
Allocator
arena_allocator(Arena self[non_null]);
INLINE
Allocator *
arena_inner_allocator(Arena *self);

AllocatorError                        
arena_alloc(Arena[non_null], usize_t, usize_t, void **);
AllocatorError
arena_resize(Arena[non_null], usize_t, usize_t, void **);
void
arena_free(Arena[non_null], void **);

#define ARENA_DEFAULT_CHUNK_SIZE 1024


#endif // CORE_ARENA_H




#if CORE_IMPL_GUARD(CORE_ARENA)
#define CORE_ARENA_I

INLINE
usize_t
arena_chunk_rest_cap(ArenaChunk *chunk) {
    return (chunk)->data_size - ((chunk)->cursor - (chunk)->data);
}
/// @brief rest capacity of the chunk after aligning the cursor
INLINE
usize_t
arena_chunk_rest_cap_aligned(ArenaChunk *chunk, usize_t alignment) {
    return (chunk)->data_size - ((u8_t *)align_forward((chunk)->cursor, alignment) - (chunk)->data);
}

/// @param[in] chunk: ArenaChunk*
/// @returns ptr to the byte **after** the last byte of data
INLINE
u8_t *
arena_chunk_end(ArenaChunk *chunk) {
    return (chunk)->data + (chunk)->data_size;
}


INLINE
usize_t
arena_chunk_count(Arena *self) {
    return self->chunks->len;
}

INLINE
void
arena_chunk_reset(ArenaChunk *self) {
    self->cursor = self->data;
}

INLINE
bool
arena_chunk_contains(ArenaChunk *self, u8_t *ptr) {
    return self->data <= ptr && ptr < arena_chunk_end(self);
}

// TODO: test this
AllocatorError
arena_init(
    Arena *self,
    usize_t default_chunk_data_size,
    Allocator *alloc)
{
    *self = (Arena) {
        .default_chunk_data_size = default_chunk_data_size,
    };
    TRY(list_new_in(alloc, typeid_of(ArenaChunk), &self->chunks));
    return ALLOCATOR_ERROR(OK);
}

void
arena_reset(Arena self[non_null]) {
    // ArenaChunk *_node_;
    for_in_list_T(ArenaChunk, chunk, self->chunks, {
        arena_chunk_reset(chunk);
    })
}

void
arena_deinit(Arena self[non_null]) {
    // for (auto node = self->head; node != nullptr; ) {
    //     auto next = node->next;
    //     allocator_free(arena_inner_allocator(self), (void**)&node);
    //     node = next;
    // }

    list_free(&self->chunks);
    *self = (Arena){ };
}



// AllocatorError                        
// _arena_allocator_alloc(Arena[non_null], usize_t , void **);
// AllocatorError
// _arena_allocator_resize(Arena[non_null] , void **, usize_t );
// void
// _arena_allocator_free(Arena[non_null], void **);

AllocatorError                        
_arena_allocator_alloc(void *self, usize_t data_size, usize_t data_align, void **out_ptr) {
    return arena_alloc((Arena *)self, data_size, data_align, out_ptr);
}

INLINE
Allocator
arena_allocator(Arena self[non_null]) {

    return (Allocator) {
        ._vtable = (Allocator_Vtable) {
            // .alloc = (AllocatorAllocFn *)arena_alloc,
            .alloc = _arena_allocator_alloc,
            .resize = (AllocatorResizeFn *)arena_resize,
            .free = (AllocatorFreeFn *)arena_free,
        },
        .data = self,
    };
}

// invalid: data_alignment is not considered
// #define arena_chunk_size(data_size) ((data_size) + sizeof(ArenaChunk))

INLINE
Allocator *
arena_inner_allocator(Arena *self) {
    return &self->chunks->alloc;
}
INLINE
usize_t
arena_total_size(Arena *self) {
    usize_t sum = 0;
    for_in_list_T(ArenaChunk, item, self->chunks, {
        sum += item->data_size;
    })
    return sum;
}

INLINE
usize_t
alignment_pad(usize_t alignment) {
    DBG_ASSERT(alignment > 0);
    return alignment - 1;
}

AllocatorError
arena_alloc(Arena self[non_null], usize_t block_size, usize_t block_align, void **out_ptr) 
{
    // Arena *_self = (Arena*)self;
    // ArenaChunk *chunk = list_find_arena_chunk(_self->head, &chunk, ___arena_allocator_alloc_pred);
    ArenaChunk NLB(*)chunk;
    #define pred(item) (arena_chunk_rest_cap_aligned(item, block_align) >= block_size)
    iter_pref_find(list, list_iter(self->chunks), pred, &chunk);
    #undef pred

    if (chunk == nullptr) {
        // TRY(allocator_alloc(arena_inner_allocator(self), sizeof(ArenaChunk) + data_section_size, alignof(ArenaChunk), (void**)&chunk));
        // TRY(alloc_sequentially_two(sizeof(ArenaChunk), alignof(ArenaChunk), data_section_size, block_align, 
        //     arena_inner_allocator(self), (void **)&chunk, &data))

        // allocate new chunk to fit the data
        usize_t data_section_size = MAX(block_size + alignment_pad(block_align), self->default_chunk_data_size);
        List_dynT_Node *node;
        list_node_alloc_size_align(self->chunks, sizeof(ArenaChunk) + data_section_size, alignof(ArenaChunk), &node);

        chunk = list_node_data(node, alignof(ArenaChunk));
        *chunk = (ArenaChunk) {
            .data_size = data_section_size,
            .cursor = chunk->data,
        };
        list_push_node(self->chunks, node);
    }
    chunk->cursor = align_forward(chunk->cursor, block_align);

    // NOTE: at this point the chunk is valid, chunk->cursor is aligned
    // insert data
    *out_ptr = chunk->cursor;
    chunk->cursor += block_size;

    return ALLOCATOR_ERROR(OK);
}

AllocatorError
arena_resize(Arena self[non_null], usize_t block_size, usize_t block_align, void **out_ptr) {
    // Arena *_self = (Arena*)self;
    unimplemented();

    // ArenaChunk *chunk;
    // LIST_FIND(_self->head, &chunk, arena_chunk_contains(_node_, *in_out_ptr));
    // if (chunk == nullptr) {
    //     TRY(arena_allocator_alloc(self, data_size, in_out_ptr));
    //     return ERROR_OK;
    // }

    // // ASSUME: chunk is valid     

    // return ERROR_OK;
}

/// doesn't deallocate memory
void
arena_free(Arena self[non_null], void **in_out_ptr) {
    *in_out_ptr = nullptr;
}

#endif // CORE_ARENA_IMPL