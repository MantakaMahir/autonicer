#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
target=${TAURI_TARGET_TRIPLE:-$(rustc -vV | awk '/host:/{print $2}')}
mkdir -p "$root/desktop/src-tauri/binaries"
make -C "$root" autonicer
cp "$root/autonicer" "$root/desktop/src-tauri/binaries/autonicer-$target"
chmod +x "$root/desktop/src-tauri/binaries/autonicer-$target"
