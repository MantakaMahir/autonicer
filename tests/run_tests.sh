#!/bin/sh
set -eu
${CC:-cc} -std=c17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Iinclude tests/test_autonicer.c src/monitor.c src/process.c src/config.c -o tests/test_autonicer
./tests/test_autonicer
./autonicer config >/dev/null
./autonicer list >/dev/null
echo "integration smoke tests passed"
