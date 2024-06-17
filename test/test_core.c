#include <criterion/criterion.h>

#define CORE_IMPL
#include "core/core.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define TEST_PRINT(a, b) printf(KYEL "\n===================\n"#a " " #b "\n\n" KNRM)

INLINE
int
foo(int32_t x) {
    return x;
}

struct Struct { int i; };
typedef struct Struct Struct;
typedef int I; // just a type allias, int == I
// typedef int NonNullInt[non_null]; not allowed
// #define NonNull(T) 

void
foo2(int i, I i2, Struct s, typeof(int [/*static*/1]) x) {}
void
foo3(usize_t size, int data[size]) {
    void 
    inner_foo3() {
        printf("stuff\n");
    }

    inner_foo3();
}
void
foo3_2(/*int data[size], usize_t size  does not work */) {
    void 
    inner_foo3() {
        printf("stuff2\n");
    }

    inner_foo3();
}

void
foo3_3(usize_t size, typeof(int[size]) data) {

}

void bar() {
    int
    bar_inner() { return 3; }
    printf("%d\n", bar_inner());
}
// void bar() {
//     int
//     g() { return 3; }
// }

void
foo4(usize_t *size) {}

void (*Foo4)(void *);

void
foo5(void *size) {}

void (*Foo5)(usize_t *);

#define G(x) _Generic((x),         \
    int: 3,                        \
    struct N: 4)                       \

struct N {};
typedef struct N2 N2;
typedef int N3; // int
// typedef double N3; // error

Test(core, c_allocator) {
    TEST_PRINT(core, c_allocator);
// int main()
    ctx_init_default();
    int *p; NEW(&p);
    // allocator_alloc(_ctx.global_alloc, sizeof(int), 1, (void**)&p);
    printf("%d\n", *p);
    *p = foo(5);
    printf("%d\n", *p);

    FREE(&p);
    // cr_assert((usize_t)p == 0);
    printf("%ld\n", (usize_t)p);
    // if (false) {
    //     return 3;
    // }
    // allocator_alloc(ctx.global_alloc.alloc, sizeof() ,&p);
}

Test(core, slice) {
    TEST_PRINT(core, slice);
    ctx_init_default();
    foo2(3, 3, (struct Struct){}, &(int){1});
    int data[3];
    foo3(3, data);
    foo3_2();
    Foo4 = (void(*)(void*))foo4;
    Foo5 = (void(*)(usize_t*))foo5;
    Foo5(&(usize_t){3}); // UB
    printf("%d\n", G(1));
}

typedef struct S2 S2;
struct S2 {
    int16_t i1;
    int8_t i2;
};
typedef struct S3 S3;
struct S3 {
    int16_t i1;
    int8_t i2;
    int16_t i3;
};
typedef struct S4 S4;
struct S4 {
    int16_t i1;
    int8_t i2;
    int32_t i3;
};
typedef struct S5 S5;
struct S5 {
    int64_t i1;
    int8_t i2;
};

typedef struct S6 S6;
struct S6 {
    // typedef int Type; no nested typedefs
    int8_t i1;
    // alignas(16) void *i2;
    double d;
};

typedef int (FooFn)(int);
FooFn *F;

#include <unistd.h>
int test_fn(int i) {
    return i*3;
}

Test(core, alignment) {
    TEST_PRINT(core, alignment);
    printf("%ld %ld\n", sizeof(S2), alignof(S2));
    printf("%ld %ld\n", sizeof(S3), alignof(S3));
    printf("%ld %ld\n", sizeof(S4), alignof(S4));
    printf("%ld %ld\n", sizeof(S5), alignof(S5));
    printf("%ld %ld\n", sizeof(S6), alignof(S6));
    printf("%ld\n", alignof(max_align_t));
    printf("%ld\n", alignof(void*));
    printf("%ld\n", alignof(3.l));
    // printf("%ld\n", sizeof(*(int *)));
}

Test(core, fn) {
    TEST_PRINT(core, fn);
    F = test_fn;
    for_in_range(i, 0, 3) {
        printf("%d\n", F(i));
    }
}

struct_def(St, {
    int i;
    int i2;
})

enum_def(En, 
    EN_A,
    EN_B,
)



extern void test_foo();

void test_foo() {
    println_fmt(S("foo"));
}

struct_decl(SFoo);
extern SFoo g_var;
// SFoo g_var;

void test_bar(SFoo sf);

extern int g_var2;
/*static*/ int g_var2;

void test_ref() {
    test_foo();
}

typedef int Foo2;
struct A {
    Foo2 Foo2;
};
// Foo2 Foo2; // error

void test_type_name() {
    struct A a;
    a.Foo2 = 3;
    // int Foo2 = 3;
    int x;

    // (a.Foo2 * Foo2);
    (Foo2) + x;
}


// Test(core, fn) {
int main() {
    ctx_init_default();
    TEST_PRINT(core, fn);
    F = test_fn;
    int x = 5;
    for_in_range(i, 0, x-i) {
        printf("%d %d\n", i, x-i);
    }

    test_ref();
}





