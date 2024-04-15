

#ifndef A_HEADER
#define A_HEADER

#include "test/test_dep/b.c"

struct A {
    int i;
};

void a_b(B b);
#endif

#ifdef A_IMPL
void
a_b(B b) {

}
#endif

