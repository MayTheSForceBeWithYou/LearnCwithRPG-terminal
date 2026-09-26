/* Intentional bad call — used by make bad. Not linked into the game. */
#include <stdio.h>
#include <stdarg.h>

#ifdef NO_FORMAT_ATTR
static void log_linef(const char *fmt, ...)
#else
static void log_linef(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)))
#endif
;

static void log_linef(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

int main(void)
{
    int x = 42;
    /* Wrong: %s expects char*, we pass int. */
    log_linef("value=%s", x);
    return 0;
}
