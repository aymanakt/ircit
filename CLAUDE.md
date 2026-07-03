# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

IRCIT 0.3.1 is a vintage (1998) text-mode IRC client for Linux, written in C by Ayman Akt. It uses ncurses for its terminal UI and includes an embedded scripting language called FPL (version 14.11). The codebase is ~37K lines of C across ~61 source files, plus the bundled FPL interpreter.

**Key constraint:** This code will only compile with GCC — it uses GNU C extensions (`__attribute__`, statement expressions, inline asm, etc.) and internal ncurses structures that have likely changed in modern ncurses versions.

## Build Commands

```bash
# First time setup
./configure          # generates Makefiles and src/include/config.h

# Build (binary lands in ./bin/ircit)
make                 # or: cd src && make

# Install (creates ~/.ircit/, copies config files, does NOT copy the binary)
make install

# Clean
make clean           # object files only
make distclean       # object files + generated Makefiles + config.*
```

The autoconf system checks for: ncurses (`-lncurses`), pthreads (`-lpthread`), GPM mouse support, and `<locale.h>`. The `config.cache` file records these results — delete it to force re-detection.

## Architecture

### Main Loop

`main.c` is minimal and shows the architecture clearly: initialize subsystems → enter infinite `while(1) { ProbeSockets(); }` loop. `ProbeSockets()` in `socketio.c` is the event pump that polls all open sockets (IRC server connections, DCC, telnet, IPC) and dispatches to protocol handlers.

**Init order** (from `main()`):
1. Parse CLI args → `InitTerminal()` (ncurses setup) → `InitInterpreter()` (FPL engine)
2. `BuildPreferencesTable()` → `InitPeople()` → `ProcessConfigfile()` → `BuildSymbolTable()`
3. `InitNet()` → `InitTimers()` → `InitSignals()` → `SessionUp()`

`InitStuffandThings()` is marked `__attribute__((constructor))` so it runs before `main()` — initializing session table, histories, sockets table, channels table, hold queue, and the window stack.

### Window / UI System (`windows.c`, `terminal.c`)

IRCIT implements its own overlapping window manager on top of ncurses. Key concepts:

- **Stack-based window management:** `stack.c` / `stack.h` — a linked list of `InputProcessor` structs. Each represents a UI "mode" (menu, dialog box, scrollable list, message box). `LoadStack()` pushes a new mode; `LoadPrevLevel()` pops back. The stack is walked to determine what's visible.
- **Window zones:** MainWin (main output), MmainWin (main border), StatusWin (lower status bar), SstatusWin, UpperStatusWin, InputWin. These are fixed ncurses windows.
- **Overlay windows:** Menus, dialog boxes, message boxes, and scroll views are transient sub-windows drawn over the main screen with save/restore of the underlying content.
- **`mt_ptr`** is the global "master terminal" pointer controlling current output state, flags, and the active window.

### IRC Protocol Handling

- **`serverio.c` / `serverr.c` / `useri.c`** (~7K lines combined) — Core IRC protocol. `serverio.c` contains the main `ProbeSockets()` loop and numeric server reply dispatcher. `serverr.c` handles named server commands (PRIVMSG, JOIN, etc.). `useri.c` handles user input commands (`/` commands).
- **`servers.c` / `servers.h`** — Multi-server connection management. Maintains a linked list of `Server` structs. Supports connecting to multiple IRC servers simultaneously.
- **`ctcp.c`** — CTCP (Client-To-Client Protocol) handling for DCC, ACTION, etc.
- **`dcc.c`** (~2.6K lines) — DCC file transfer and chat support.
- **`telnet.c` / `telnetcmds.c`** — Telnet protocol support for connecting to non-IRC services.

### Networking (`net.c`, `sockets.c`, `socketio.c`)

- `net.c` — DNS resolution (blocking, with cache in `dnscache.c`), address parsing, connection establishment. Uses raw `gethostbyname()` etc.
- `sockets.c` — Socket table management (linked list of all open connections). `RequestSocket()` allocates a slot.
- `socketio.c` — The `select()`-based event loop (`ProbeSockets()`). Reads from active sockets and routes data to the appropriate protocol handler (IRC, DCC, telnet, or IPC).

### FPL Scripting Language

FPL is an embedded scripting language (separate codebase by "Daniel") located at `src/fpl-14.12/`. It provides:
- A C-like scripting syntax for IRCIT automation
- `fpl.c` — IRCIT's bridge between FPL and the rest of the application. Defines intrinsics (built-in functions like `say()`, `send()`, `join()`) and event bindings.
- Config files (`~/.ircit/ircit.fpl`) are FPL scripts that IRCIT loads at startup.
- `intrins.c` / `intrins.h` — Built-in FPL functions exposed to scripts.
- `prefs.c` / `prefsfunc.c` — The preferences system (SET variables) with FPL integration.

### Channel & User State

- `chanrec.c` — Channel records (joined channels, topics, modes).
- `chanusers.c` — Per-channel user lists with nick tracking.
- `people.c` — Global user tracking across all channels.
- `ignore.c` — User ignore/highlight system.
- `notify.c` — Watch-list for user online/offline notifications.

### Input Handling

`input.c` — Keyboard input processing. Dispatches between:
- Regular text input (sent as channel messages)
- `/` commands (useri.c dispatch)
- FPL expressions (prefixed with certain characters)
- Mouse events (in `mouse.c`, via GPM or xterm mouse)

### Other Notable Files

- **`configfile.c`** — Parses `~/.ircit/ircit.conf` (INI-like format). The config file sets preferences, server lists, and auto-join channels.
- **`history.c`** — Command history with persistence.
- **`output.c`** — Formatted output to windows with color/attribute support. `nYell()` / `lnYell()` are the main output primitives (variadic, format-string based).
- **`colors.c` / `colorstab.h`** — Color theme definitions and the object-to-color mapping table.
- **`timer.c`** — Timed event system (used for auto-reconnect, flood protection delays, etc.).
- **`flood.c`** — Anti-flood protection with configurable thresholds.
- **`soundsrv.c` / `sounds.c`** — Sound server / beep-on-message support.
- **`thread.c`** — Thread creation using the Linux `clone()` syscall directly (not pthreads, despite pthreads being detected by configure).
- **`exec.c`** — Fork/exec external programs and capture output.
- **`match.c`** — Wildcard matching (IRC-style `*` and `?` patterns).
- **`str.c` / `mvsprintf.c`** — Custom string utilities including a `vsprintf` variant.

### Header Structure

All headers are in `src/include/`. The key dependency chain:
- `main.h` — Universal include (pulled in by nearly everything). Defines core types, macros (`xmalloc`, `xfree`, `say()`, `swprintf()`), buffer size constants, and the `STD_*` command typedefs.
- `terminal.h` — ncurses includes, window declarations, color enums, ANSI drawing macros.
- `config.h` (generated by `configure`) — Platform feature detection results.
- `headers.h` — Static header strings for 20 different list/table views (servers, channels, DCC, timers, etc.).

Some headers come in `-data` pairs (e.g., `servers.h` + `servers-data.h`) — the `-data` variant contains static table/menu definition data.

### IPC

IRCIT supports inter-process communication via Unix domain sockets (the `IPC_PATH` config variable). `InitIPC()` in `net.c` sets up a listener. This allows external tools to send commands to a running IRCIT instance.

## Configuration Files

At startup, IRCIT reads (if present):
- `~/.ircit/ircit.conf` — preferences and server definitions
- `~/.ircit/ircit.fpl` — FPL script with aliases, event bindings, and automation

## Documentation

- `doc/ircit.help` (38K) — Main help file with documented commands
- `doc/ircit.1` — Man page
- `doc/programming/` — FPL scripting guide, event list, internal functions reference
- `doc/NEWS` — Release changelog
- `doc/BUGS` — Known bugs
