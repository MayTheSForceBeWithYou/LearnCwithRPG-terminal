/* Drill - format contract. Do NOT strip attribute from production log_linef. */
#include <stdio.h>
#include <stdarg.h>

static void log_linef(const char *fmt, ...)
#if defined(__GNUC__)
    /* TODO: replace FORMAT_ATTR_ARGS with (printf, 1, 2) */
    __attribute__((format FORMAT_ATTR_ARGS))
#endif
;

static void log_linef(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc(10, stdout);
}

int main(void)
{
    log_linef("hero hp=%d name=%s", 30, "Ada");
    puts("format_contract: good call compiled and ran");
    puts("Next: make bad  (compares with/without attribute on a bad call)");
    return 0;
}
