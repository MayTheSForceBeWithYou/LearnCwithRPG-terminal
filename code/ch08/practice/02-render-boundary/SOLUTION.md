# Solution

In `draw_world`:

```c
static void draw_world(void)
{
    render_draw('@');
    render_present();
}
```

Calling `fake_refresh()` (or, in the real game, `refresh()`) from game code
couples you to the backend. Today it looks identical; tomorrow an SDL port
has to chase every call site. Keep the seam.
