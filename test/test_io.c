
#include <criterion/criterion.h>
#define CORE_IMPL
#include "core/io.h"
#include "core/string.h"

// Test(io, output_file_stream) {
int main() {
    ctx_init_default();
    auto s = S(KRED "help!\n");
    output_file_stream_write(&g_stdout_ofs, s.byte_len, (u8_t *)s.ptr);
    output_file_stream_flush(&g_stdout_ofs);
    stream_writer_write(&g_ctx.stdout_sw, s.byte_len, (u8_t *)s.ptr);
    stream_writer_flush(&g_ctx.stdout_sw);
}
