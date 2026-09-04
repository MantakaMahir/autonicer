#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
if [ -z "${TAURI_TARGET_TRIPLE:-}" ]; then
    if ! command -v rustc >/dev/null 2>&1; then
        printf '%s\n' 'Rust is required. Install it with: curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh' >&2
        exit 1
    fi
    target=$(rustc -vV | awk '/host:/{print $2}')
else
    target=$TAURI_TARGET_TRIPLE
fi
mkdir -p "$root/desktop/src-tauri/binaries"
make -C "$root" autonicer
cp "$root/autonicer" "$root/desktop/src-tauri/binaries/autonicer-$target"
chmod +x "$root/desktop/src-tauri/binaries/autonicer-$target"
