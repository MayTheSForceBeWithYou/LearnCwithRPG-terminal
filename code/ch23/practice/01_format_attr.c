#include <stdio.h>
#include <stdarg.h>
#if defined(__GNUC__)
/* TODO: complete the format attribute arguments (printf-style, fmt is arg 1) */
__attribute__((format FORMAT_ATTR_ARGS))
#endif
static void log_linef(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc('\n', stdout);
}
int main(void)
{
    log_linef("hp=%d", 20);
    /* After FORMAT_ATTR_ARGS is correct, log_linef("%s", 20) warns under -Wformat. */
    puts("01_format_attr: all checks passed");
    return 0;
}
