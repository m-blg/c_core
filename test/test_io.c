#define CORE_IMPL
#include "core/io.h"
#include "core/string.h"
#include <unistd.h>

void
test1() {
    auto s = S(KRED "help!\n");
    output_file_stream_write(&g_stdout_ofs, s.byte_len, (u8_t *)s.ptr);
    output_file_stream_flush(&g_stdout_ofs);
    stream_writer_write(&g_ctx.stdout_sw, s.byte_len, (u8_t *)s.ptr);
    stream_writer_flush(&g_ctx.stdout_sw);
}

AllocatorError
os_cwd(Allocator *alloc, str_t *out_str) {
    getcwd(g_ctx.dump_buffer.ptr, slice_len(&g_ctx.dump_buffer));
    TRY(str_from_c_str_in(g_ctx.dump_buffer.ptr, alloc, out_str));
    return ALLOCATOR_ERROR(OK);
}

void
test2() {
    str_t s;
    os_cwd(&g_ctx.imm_str_alloc, &s);
    println_fmt(s);
}

int main() {
    ctx_init_default();

    // test1();
    test2();
}
