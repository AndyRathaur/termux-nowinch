# termux-nowinch

A lightweight terminal window-resize (`SIGWINCH`) filter proxy for **Android Termux**.

It prevents annoying full-screen conversation redraws, scrolling animations, and lag in interactive CLI TUIs like **Google Antigravity CLI (`agy`)** whenever the virtual keyboard opens or closes, or when switching between apps.

---

## The Problem

When using `agy` (or other interactive terminal UIs) in Android Termux:

1. **Software Keyboard Toggles**: Showing or hiding the keyboard changes the visible terminal height (`rows`).
2. **App Switching**: Switching to another app and returning to Termux re-evaluates viewport dimensions.
3. **The Lag**: In standard inline mode, every resize causes `SIGWINCH` to be fired. The CLI recalculates layout and flushes hundreds of lines of styled ANSI escape codes into the terminal scrollback buffer. On a phone, this causes the screen to slowly scroll through the entire chat history from top to bottom, freezing the app for several seconds.

While enabling `altScreenMode` stops the scroll lag, it sacrifices native touch-swipe scrolling and standard terminal scrollback in Termux.

---

## The Solution: `nowinch`

`nowinch` acts as a transparent, high-performance PTY proxy between Termux and the target command:

- **Filters Height-Only Resizes**: Opening/closing the software keyboard or switching apps changes only the terminal **rows**, while **columns (width)** remain unchanged. `nowinch` catches `SIGWINCH` and drops row-only changes. The child application (`agy`) never receives `SIGWINCH`, preventing all redraw lag!
- **Preserves Orientation Changes**: If you rotate your phone between portrait and landscape, the column count changes. `nowinch` forwards the resize so lines re-wrap cleanly.
- **Maintains 100% Native Scrolling**: Runs in standard terminal inline mode. All output is kept in Termux's native scrollback buffer so you can smoothly swipe up and down with your finger.

---

## Quick Install (One-Liner)

Run the following command directly inside Termux:

```bash
curl -fsSL https://raw.githubusercontent.com/AndyRathaur/termux-nowinch/main/install.sh | bash
```

Once installed, simply start Antigravity as usual:
```bash
agy
```

---

## Manual Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/AndyRathaur/termux-nowinch.git
   cd termux-nowinch
   ```

2. Compile and install:
   ```bash
   make
   make install
   ```

3. Wrap `agy`:
   ```bash
   alias agy="nowinch /data/data/com.termux/files/usr/bin/agy"
   ```

---

## Usage with Other Tools

`nowinch` can be used with any CLI tool that misbehaves or flickers upon keyboard resize in Termux:

```bash
nowinch <command> [args...]

# Examples:
nowinch agy
nowinch python3
```

---

## Uninstallation

To remove `nowinch` and restore the default behavior:

```bash
curl -fsSL https://raw.githubusercontent.com/AndyRathaur/termux-nowinch/main/uninstall.sh | bash
```
Or via Makefile:
```bash
make uninstall
```

---

## License

[MIT](LICENSE) © 2026 Andy Rathaur
