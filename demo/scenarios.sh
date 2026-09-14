#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
STATE_DIR=${XDG_STATE_HOME:-"$HOME/.local/state"}/autonicer
PID_FILE=$STATE_DIR/demo-pids
mkdir -p "$STATE_DIR"

usage() {
    cat <<'EOF'
Usage: ./demo/scenarios.sh COMMAND

Commands:
  start-cpu        Start one CPU workload as a safe background candidate
  start-memory     Start one 256 MiB memory workload
  start-all        Start CPU, protected CPU, and memory workloads
  status           Show tracked workloads and current process data
  stop             Stop all tracked workloads safely
  cleanup          Alias for stop
EOF
}

require_binaries() {
    if [ ! -x "$ROOT/demo/cpu_hog" ] || [ ! -x "$ROOT/demo/memory_hog" ]; then
        printf '%s\n' 'Demo binaries are missing. Run: make' >&2
        exit 1
    fi
}

is_demo_process() {
    pid=$1
    expected=$2
    [ -r "/proc/$pid/comm" ] || return 1
    [ "$(tr -d '\n' < "/proc/$pid/comm" 2>/dev/null)" = "$expected" ]
}

record() {
    printf '%s %s %s\n' "$1" "$2" "$3" >> "$PID_FILE"
}

start_workload() {
    kind=$1
    name=$2
    output=$3
    shift 3
    "$ROOT/demo/$name" "$@" >"$STATE_DIR/$output.log" 2>&1 &
    pid=$!
    sleep 0.1
    if ! is_demo_process "$pid" "$name"; then
        printf 'Failed to start %s. See %s.\n' "$name" "$STATE_DIR/$output.log" >&2
        kill "$pid" 2>/dev/null || true
        exit 1
    fi
    record "$pid" "$kind" "$name"
    printf '%s PID %s started (%s)\n' "$name" "$pid" "$kind"
    printf '  log: %s\n' "$STATE_DIR/$output.log"
}

start_cpu() {
    start_workload background cpu_hog cpu-hog
    pid=$(awk 'END { print $1 }' "$PID_FILE")
    "$ROOT/autonicer" classify "$pid" background
}

start_memory() {
    start_workload memory memory_hog memory-hog 256
}

start_all() {
    start_workload background cpu_hog cpu-background
    background_pid=$(awk 'END { print $1 }' "$PID_FILE")
    "$ROOT/autonicer" classify "$background_pid" background

    start_workload protected cpu_hog cpu-protected
    protected_pid=$(awk 'END { print $1 }' "$PID_FILE")
    "$ROOT/autonicer" classify "$protected_pid" critical
    "$ROOT/autonicer" protect "$protected_pid"

    start_workload memory memory_hog memory-hog 256
    printf '%s\n' 'Mixed scenario ready: one BACKGROUND CPU candidate, one protected CRITICAL CPU process, and one memory workload.'
}

status() {
    if [ ! -s "$PID_FILE" ]; then
        printf '%s\n' 'No tracked demo workloads.'
        exit 0
    fi
    printf '%-8s %-12s %-14s %s\n' PID ROLE PROCESS STATE
    while read -r pid role name; do
        [ -n "$pid" ] || continue
        if is_demo_process "$pid" "$name"; then
            printf '%-8s %-12s %-14s running\n' "$pid" "$role" "$name"
        else
            printf '%-8s %-12s %-14s stopped\n' "$pid" "$role" "$name"
        fi
    done < "$PID_FILE"
    printf '\nRegistered process state:\n'
    "$ROOT/autonicer" list --json
}

stop_all() {
    [ -f "$PID_FILE" ] || exit 0
    tmp="$PID_FILE.tmp.$$"
    : > "$tmp"
    while read -r pid role name; do
        [ -n "$pid" ] || continue
        if is_demo_process "$pid" "$name"; then
            "$ROOT/autonicer" restore "$pid" >/dev/null 2>&1 || true
            kill "$pid" 2>/dev/null || true
            i=0
            while is_demo_process "$pid" "$name" && [ "$i" -lt 20 ]; do
                sleep 0.1
                i=$((i + 1))
            done
            if is_demo_process "$pid" "$name"; then
                kill -KILL "$pid" 2>/dev/null || true
            fi
            printf 'Stopped %s PID %s\n' "$name" "$pid"
        fi
    done < "$PID_FILE"
    rm -f "$PID_FILE" "$tmp"
}

[ "$#" -eq 1 ] || { usage; exit 2; }
require_binaries
case "$1" in
    start-cpu) start_cpu ;;
    start-memory) start_memory ;;
    start-all) start_all ;;
    status) status ;;
    stop|cleanup) stop_all ;;
    -h|--help) usage ;;
    *) usage >&2; exit 2 ;;
esac
