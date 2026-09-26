/* Drill - format contract. Do NOT strip attribute from production log_linef. */
#include <stdio.h>
#include <stdarg.h>

static void log_linef(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

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
