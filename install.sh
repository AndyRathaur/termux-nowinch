#!/data/data/com.termux/files/usr/bin/bash
set -e

echo "==> Installing nowinch (Termux window-resize suppressor)..."

# Ensure clang is installed
if ! command -v clang >/dev/null 2>&1; then
    echo "==> clang not found, installing..."
    pkg update -y && pkg install -y clang
fi

PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"
BINDIR="$PREFIX/bin"
SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Compile nowinch
echo "==> Compiling nowinch..."
clang -O2 "$SRC_DIR/nowinch.c" -o "$BINDIR/nowinch"
chmod 755 "$BINDIR/nowinch"
echo "==> Installed nowinch to $BINDIR/nowinch"

# Set up Antigravity wrapper if agy is present
AGY_WRAPPER_DIR="$HOME/.gemini/antigravity-cli/bin"
if [ -d "$HOME/.gemini/antigravity-cli" ]; then
    mkdir -p "$AGY_WRAPPER_DIR"
    cat << 'EOF' > "$AGY_WRAPPER_DIR/agy"
#!/data/data/com.termux/files/usr/bin/bash
exec /data/data/com.termux/files/usr/bin/nowinch /data/data/com.termux/files/usr/bin/agy "$@"
EOF
    chmod 755 "$AGY_WRAPPER_DIR/agy"
    echo "==> Configured agy wrapper at $AGY_WRAPPER_DIR/agy"
fi

# Set up alias in ~/.bashrc
BASHRC="$HOME/.bashrc"
if ! grep -q "nowinch" "$BASHRC" 2>/dev/null; then
    echo "" >> "$BASHRC"
    echo "# Antigravity (agy) window-resize suppressor for Termux" >> "$BASHRC"
    echo "alias agy=\"nowinch /data/data/com.termux/files/usr/bin/agy\"" >> "$BASHRC"
    echo "==> Added agy alias to $BASHRC"
fi

# Ensure ~/.bash_profile sources ~/.bashrc
BASH_PROFILE="$HOME/.bash_profile"
if [ ! -f "$BASH_PROFILE" ] || ! grep -q "bashrc" "$BASH_PROFILE" 2>/dev/null; then
    cat << 'EOF' >> "$BASH_PROFILE"
if [ -f ~/.bashrc ]; then
    . ~/.bashrc
fi
EOF
    echo "==> Ensured $BASH_PROFILE sources ~/.bashrc"
fi

echo ""
echo "=========================================================="
echo "  nowinch installed successfully!"
echo "  Run 'agy' to use Antigravity without resize redraw lag."
echo "=========================================================="
