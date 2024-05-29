#define CORE_IMPL
#include "core/arena.h"


void
test1()
{
    Arena arena; arena_init(&arena, 0x400, &g_ctx.global_alloc);
    
    usize_t *var = nullptr;
    ASSERT_OK(arena_alloc(&arena, 0x1000, alignof(*var), (void **)&var));

    *var = 3;
    printf("%ld" "\n", *var);
    printf("%ld" "\n", var[0x10]);
    arena_free(&arena, (void**)&var);
    
    arena_deinit(&arena);
}

void
test2()
{
    Arena arena; arena_init(&arena, 0x4, &g_ctx.global_alloc);
    Allocator alloc = arena_allocator(&arena);
    
    DBG_ASSERT(alloc.data == &arena);
    usize_t *var = nullptr;
    // allocator_alloc_z(&alloc, 0x1000, alignof(*var), (void **)&var);
    // allocator_alloc_T(&alloc, usize_t, (void **)&var);
    allocator_alloc_zn(&alloc, usize_t, 0x1000, (void**)&var);
    // printf("%ld" "\n", *var);
    printf("%ld" "\n", var[0x10]);
    allocator_free(&alloc, (void**)&var);
    
    arena_deinit(&arena);
}
void
test3()
{
    Arena arena; arena_init(&arena, 0x4, &g_ctx.global_alloc);
    Allocator alloc = arena_allocator(&arena);
    
    DBG_ASSERT(alloc.data == &arena);
    usize_t *var[3];
    for_in_range(i, 0, 3) {
        ASSERT_OK(allocator_alloc_z(&alloc, 0x8, alignof(*var), (void **)&var[i]));
        *var[i] = i;
    }
    for_in_range(i, 0, 3) {
        printf("%ld" "\n", *var[i]);
    }
    allocator_free(&alloc, (void**)&var);
    
    arena_deinit(&arena);
}


int 
main()
{
    ctx_init_default();
    
    test1();
    test2();
    test3();
}