# Appendix A: WSL Troubleshooting

A collected reference for problems specific to developing inside WSL2 running
Arch Linux. Chapters cross-reference this appendix instead of repeating it.

---

## "No window appears" (graphical track only)

Not relevant to the ncurses track chosen in this course (Checkpoint A), but
if you later migrate to SDL2 (see Chapter 8's renderer abstraction):

1. Confirm `$DISPLAY` is set: `echo $DISPLAY`. It should print something
   like `:0`. If empty, WSLg isn't wired up for this session.
2. From PowerShell on the Windows side, run `wsl --update`, then fully
   restart WSL (`wsl --shutdown` from PowerShell, then reopen your terminal).
   This fixes a large fraction of "no window appears" reports — WSLg ships
   as part of the WSL platform, not the Linux distro, so it updates outside
   `pacman`.
3. Test with a trivial SDL2 window before touching any tutorial code, to
   isolate "my environment is broken" from "my code is broken."

## Terminal rendering looks wrong (ncurses track)

- **Windows Terminal** (the modern one, not the legacy `conhost.exe` window)
  handles color and Unicode box-drawing characters correctly. The legacy
  console host frequently does not — if your map borders look like garbage
  characters instead of lines, this is almost always why.
- Check `$TERM`: run `echo $TERM`. It should be something like
  `xterm-256color`. If it prints `dumb` or is empty, ncurses will degrade to
  the least capable rendering it knows how to do. Set it explicitly in your
  shell's startup file (`~/.zshrc` or `~/.bashrc`) if needed:
  ```bash
  export TERM=xterm-256color
  ```

## `pacman-key --init` / keyring failures

Freshly imported Arch rootfs images (including many WSL Arch installers)
sometimes have an uninitialized or stale package signing keyring, producing
errors like:

```
error: key "..." could not be looked up remotely
error: unable to synchronize with any keyservers
```

Fix:

```bash
sudo pacman-key --init
sudo pacman-key --populate archlinux
sudo pacman -Syu
```

## Clock skew breaking TLS during `pacman -Sy`

WSL2 VMs occasionally drift out of sync with host time, especially after the
Windows host sleeps/resumes. This breaks TLS certificate validation during
`pacman -Sy` with errors mentioning certificate validity or "not yet valid" /
"has expired". Fix by resyncing the clock:

```bash
sudo timedatectl set-ntp true
# If that doesn't help immediately:
sudo hwclock -s
```

If neither works, a full `wsl --shutdown` from PowerShell and reopening the
terminal usually resyncs the VM clock against the host.

## Stale kernel symptoms

WSL2 uses a single Linux kernel shared across all your distros, managed by
Windows, not by `pacman`. If you see kernel/filesystem-driver-looking errors
that make no sense given an up-to-date Arch install (`pacman -Syu` clean but
something's still broken at a low level), suspect a stale WSL kernel before
anything else:

```powershell
# From PowerShell (not WSL):
wsl --update
wsl --shutdown
```

Then reopen your Arch terminal.

## Audio (optional sound chapter only)

Audio routes through WSLg's PulseAudio bridge and generally works without
configuration on current WSL builds. If you attempt the optional sound
chapter and hear nothing, this is the most fragile part of the stack — check
`wsl --update` first, same as the graphical-window issue above.

## Valgrind: "Fatal error at startup: a function redirection"

Introduced in Chapter 22. You install `valgrind`, run it, and it refuses to
start at all:

```
valgrind:  Fatal error at startup: a function redirection
valgrind:  which is mandatory for this platform-tool combination
valgrind:  cannot be set up.  Details of the redirection are:
valgrind:
valgrind:  A must-be-redirected function
valgrind:  whose name matches the pattern:      memcmp
valgrind:  in an object with soname matching:   ld-linux-x86-64.so.2
valgrind:  was not found whilst processing
valgrind:  symbols from the object with soname: ld-linux-x86-64.so.2
valgrind:
valgrind:  Possible fixes: (1, short term): install glibc's debuginfo
valgrind:  package on this machine.
```

Valgrind needs debug symbols for the dynamic linker, and Arch ships a
stripped `ld.so`. The suggested fix names Debian and Fedora packages —
`libc6-dbg`, `glibc-debuginfo` — and **neither exists on Arch**. There is no
`glibc-debug` package in the Arch repositories either:

```
error: package 'glibc-debug' was not found
```

Arch serves debug symbols over the network instead, through **debuginfod**.
Valgrind 3.20 and later can fetch what it needs from there, and it is
already configured — `/etc/debuginfod/archlinux.urls` ships with the
`debuginfod` package and is exported into your environment by
`/etc/profile.d/debuginfod.sh`.

The catch is that the profile script runs at **login**. If you installed
`valgrind` and `debuginfod` during your current session, the variable is not
set in that shell yet. Check it:

```bash
echo $DEBUGINFOD_URLS
```

If that prints nothing, either open a new terminal, or set it for one
command:

```bash
DEBUGINFOD_URLS=https://debuginfod.archlinux.org valgrind ./run_tests
```

Both work. Once the variable is set, Valgrind starts normally.

Two consequences worth knowing:

- **The first run needs network access**, because the symbols are
  downloaded rather than installed. They are cached under
  `~/.cache/debuginfod_client` afterwards, so later runs work offline.
- **The first run is slow** while that download happens — on top of
  Valgrind's usual 20–50× slowdown. Don't conclude your program has hung.

This also affects `gdb`, which uses the same mechanism to fetch symbols for
system libraries.
