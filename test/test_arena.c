#include <criterion/criterion.h>
#include "core/arena.c"



Test(arena, c_alloc)
{
    ctx_init_default();
    
    usize_t *var = nullptr;
    allocator_alloc_zn(_ctx.global_alloc, usize_t, 0x1000, (void**)&var);
    printf("%ld" "\n", var[0x10]);
    allocator_free(_ctx.global_alloc, (void**)&var);
    
}

// Test(arena, basic)
int main()
{
    ctx_init_default();
    Arena arena; arena_init(&arena, 0x400, _ctx.global_alloc);
    Allocator *alloc = arena_allocator(&arena);
    
    usize_t *var = nullptr;
    allocator_alloc_zn(alloc, usize_t, 0x1000, (void**)&var);
    printf("%ld" "\n", var[0x10]);
    allocator_free(alloc, (void**)&var);
    
    arena_free(&arena);
}