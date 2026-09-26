#include <stdio.h>
#include "render_stub.h"

static int presents;
static char last;

static void backend_flush(void)
{
    presents++;
    /* In real life this is refresh(). The counter proves the seam was used. */
    (void)last;
}

void render_draw(char cell)
{
    last = cell;
}

void render_present(void)
{
    backend_flush();
}

/*
 * Backend name — not declared in render_stub.h.
 * EXPOSE_FAKE_REFRESH makes it linkable for the reading-only leak target.
 */
#ifdef EXPOSE_FAKE_REFRESH
void fake_refresh(void)
{
    backend_flush();
}
#endif

int render_present_count(void)
{
    return presents;
}
