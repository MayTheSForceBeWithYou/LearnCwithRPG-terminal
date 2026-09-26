#include <stdio.h>
#include <stdarg.h>
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
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
    /* Uncomment to see -Wformat: log_linef("%s", 20); */
    puts("01_format_attr: all checks passed");
    return 0;
}
