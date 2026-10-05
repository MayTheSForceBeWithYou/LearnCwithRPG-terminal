#include <stdio.h>
#include <stdint.h>

/* Predict struct sizes, then fill EXPECT_* so the checks compile and pass. */

typedef struct {
    char a;
    int b;
    char c;
} Naive;

typedef struct {
    int b;
    char a;
    char c;
} Packed;

typedef struct {
    uint8_t a;
    int32_t b;
    uint8_t c;
} FixedWidth;

static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }

int main(void)
{
    /* TODO: replace each EXPECT_*_VALUE with your predicted size in bytes */
    enum {
        EXPECT_NAIVE = EXPECT_NAIVE_VALUE,
        EXPECT_PACKED = EXPECT_PACKED_VALUE,
        EXPECT_FIXED = EXPECT_FIXED_VALUE
    };

    printf("sizeof(Naive)=%zu (expect %d)\n", sizeof(Naive), EXPECT_NAIVE);
    printf("sizeof(Packed)=%zu (expect %d)\n", sizeof(Packed), EXPECT_PACKED);
    printf("sizeof(FixedWidth)=%zu (expect %d)\n", sizeof(FixedWidth), EXPECT_FIXED);
    printf("offset Naive.b=%zu Packed.a=%zu\n",
           __builtin_offsetof(Naive, b), __builtin_offsetof(Packed, a));

    check(sizeof(Naive) == (size_t)EXPECT_NAIVE, "Naive size");
    check(sizeof(Packed) == (size_t)EXPECT_PACKED, "Packed size");
    check(sizeof(FixedWidth) == (size_t)EXPECT_FIXED, "FixedWidth size");
    check(sizeof(Packed) < sizeof(Naive), "reordering shrinks padding");

    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("05_sizeof_alignment: all checks passed");
    return 0;
}
