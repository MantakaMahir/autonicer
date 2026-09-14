#!/bin/sh
set -eu
${CC:-cc} -std=c17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Iinclude tests/test_autonicer.c src/monitor.c src/process.c src/config.c src/memory.c -o tests/test_autonicer
./tests/test_autonicer
./autonicer config >/dev/null
./autonicer list >/dev/null
./autonicer list --all --json >/tmp/autonicer-processes.json
./autonicer memory --json >/tmp/autonicer-memory.json
./autonicer pager-demo --algorithm fifo --frames 3 --reference '1,2,3,1W,4,2' >/tmp/autonicer-pager.json
./autonicer pager-demo --algorithm lru --frames 3 --reference '1,2,3,1W,4,2' >/tmp/autonicer-lru.json
./autonicer pager-demo --algorithm clock --frames 3 --reference '1,2,3,1W,4,2' >/tmp/autonicer-clock.json
python3 -c 'import json; m=json.load(open("/tmp/autonicer-memory.json")); p=json.load(open("/tmp/autonicer-processes.json")); results=[json.load(open(x)) for x in ("/tmp/autonicer-pager.json","/tmp/autonicer-lru.json","/tmp/autonicer-clock.json")]; assert "availableKb" in m and {"available","someAvg10","fullAvg10"} <= m["psi"].keys() and all(x["name"] not in {"systemd","sd-pam","dbus-daemon","pipewire","wireplumber"} for x in p) and [x["algorithm"] for x in results] == ["fifo","lru","clock"] and all(x["faults"] > 0 and len(x["steps"]) == 6 for x in results)'
echo "integration smoke tests passed"
