#include <stdio.h>

/* Explore sizeof, demonstrate struct padding, and show why smaller types
   don't always save space. */

/* A naive struct: smallest types for each field. */
typedef struct {
    char a;      /* 1 byte */
    int b;       /* 4 bytes */
    char c;      /* 1 byte */
} Naive;

/* Same fields, different order: largest-to-smallest. */
typedef struct {
    int b;       /* 4 bytes */
    char a;      /* 1 byte */
    char c;      /* 1 byte */
} Packed;

/* Using uint8_t doesn't change struct layout meaningfully here. */
#include <stdint.h>
typedef struct {
    uint8_t a;
    int32_t b;
    uint8_t c;
} FixedWidth;

int main(void)
{
    printf("Basic type sizes:\n");
    printf("  sizeof(char):        %zu byte\n", sizeof(char));
    printf("  sizeof(int):         %zu bytes\n", sizeof(int));
    printf("  sizeof(unsigned):    %zu bytes\n", sizeof(unsigned));
    printf("  sizeof(float):       %zu bytes\n", sizeof(float));
    printf("  sizeof(double):      %zu bytes\n", sizeof(double));
    printf("\n");

    printf("Fixed-width types:\n");
    printf("  sizeof(uint8_t):     %zu byte\n", sizeof(uint8_t));
    printf("  sizeof(uint16_t):    %zu bytes\n", sizeof(uint16_t));
    printf("  sizeof(uint32_t):    %zu bytes\n", sizeof(uint32_t));
    printf("  sizeof(int32_t):     %zu bytes\n", sizeof(int32_t));
    printf("\n");

    printf("Struct padding:\n");
    printf("  sizeof(Naive):       %zu bytes  (char, int, char)\n", sizeof(Naive));
    printf("  sizeof(Packed):      %zu bytes  (int, char, char)\n", sizeof(Packed));
    printf("  sizeof(FixedWidth):  %zu bytes  (uint8_t, int32_t, uint8_t)\n", sizeof(FixedWidth));
    printf("\n");

    /* The moral: Naive is 12 bytes (3 bytes padding after a, 3 after c).
       Packed is 8 bytes (2 bytes padding after a and c together).
       Ordering matters; type width alone does not determine struct size. */

    printf("Memory layout of Naive (char a, int b, char c):\n");
    printf("  Offset of a: %zu\n", __builtin_offsetof(Naive, a));
    printf("  Offset of b: %zu  (3 bytes padding after a)\n", __builtin_offsetof(Naive, b));
    printf("  Offset of c: %zu  (right after b)\n", __builtin_offsetof(Naive, c));
    printf("  Total size:  %zu  (3 bytes padding at end)\n", sizeof(Naive));
    printf("\n");

    printf("Memory layout of Packed (int b, char a, char c):\n");
    printf("  Offset of b: %zu\n", __builtin_offsetof(Packed, b));
    printf("  Offset of a: %zu\n", __builtin_offsetof(Packed, a));
    printf("  Offset of c: %zu\n", __builtin_offsetof(Packed, c));
    printf("  Total size:  %zu  (2 bytes padding at end)\n", sizeof(Packed));
    printf("\n");

    printf("Takeaway: using smaller types in a single local variable saves\n");
    printf("nothing (promotion + register allocation). In a struct or array\n");
    printf("with many elements, layout and ordering matter more than raw width.\n");

    return 0;
}
