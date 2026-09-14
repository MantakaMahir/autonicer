#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
DESKTOP="$ROOT/desktop"

# Remove only stale processes from this checkout so Vite can bind its fixed dev port.
pkill -f "$DESKTOP/node_modules/.bin/vite" 2>/dev/null || true
pkill -f "$DESKTOP/src-tauri/target/debug/autonicer-desktop" 2>/dev/null || true

if ! command -v node >/dev/null 2>&1; then
    printf '%s\n' 'Node.js is required.' >&2
    exit 1
fi

if ! command -v cargo >/dev/null 2>&1; then
    if [ -f "$HOME/.cargo/env" ]; then
        . "$HOME/.cargo/env"
    fi
fi

if ! command -v cargo >/dev/null 2>&1; then
    printf '%s\n' 'Cargo is required. Install Rust with: curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh' >&2
    exit 1
fi

if [ ! -d "$DESKTOP/node_modules" ]; then
    npm install --prefix "$DESKTOP"
fi

make -C "$ROOT" autonicer
exec npm run tauri:dev --prefix "$DESKTOP"
