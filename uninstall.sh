#!/data/data/com.termux/files/usr/bin/bash
set -e

echo "==> Uninstalling nowinch..."

PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"
rm -f "$PREFIX/bin/nowinch"

if [ -f "$HOME/.gemini/antigravity-cli/bin/agy" ]; then
    rm -f "$HOME/.gemini/antigravity-cli/bin/agy"
    echo "==> Removed agy wrapper script"
fi

if [ -f "$HOME/.bashrc" ]; then
    sed -i '/nowinch/d' "$HOME/.bashrc"
    sed -i '/Antigravity (agy) window-resize suppressor/d' "$HOME/.bashrc"
    echo "==> Removed nowinch alias from ~/.bashrc"
fi

echo "==> nowinch uninstalled successfully."
