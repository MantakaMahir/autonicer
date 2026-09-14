# AutoNicer

AutoNicer is a Linux C17 controller and Tauri desktop application for observing CPU and memory pressure and safely managing explicitly selected background processes. It reads Linux telemetry from `/proc`, makes policy decisions in the C core, and exposes the same data through a terminal CLI and desktop UI.

It does not kill processes, execute arbitrary shell commands, modify kernel page tables, or use a web backend.

## Quick Start

```sh
make
make test
./run-desktop.sh
```

The desktop launcher builds the C core, starts the Tauri development window, and removes only stale Vite/Tauri processes belonging to this checkout. Use the Tauri window, not `npm run dev`, for live core data.

## Requirements

- Linux with `/proc` mounted
- C compiler with C17 support and `make`
- Node.js and npm for the desktop UI
- Rust and Cargo for Tauri
- Tauri Linux dependencies: GTK, WebKitGTK, and their development packages

Install Rust if necessary:

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
. "$HOME/.cargo/env"
```

The frontend can be built with Node alone, but process controls and live Linux telemetry require Linux, the compiled C binary, Rust, and Tauri.

## Build And Test

Core build and tests:

```sh
make                 # core and demo workloads
make test            # unit and integration tests
make clean
```

Desktop build and checks:

```sh
make desktop-install
make desktop-build
npm run build --prefix desktop
cargo check --manifest-path desktop/src-tauri/Cargo.toml
```

Desktop development commands:

```sh
./run-desktop.sh
npm run tauri:dev --prefix desktop
npm run tauri:build --prefix desktop
```

`desktop/package.json` also provides `dev` for a browser-only Vite preview. That preview cannot call Tauri IPC and is not a functional AutoNicer desktop session.

## CLI Commands

Build first with `make`, then run commands from the repository root.

```text
./autonicer sample
./autonicer list
./autonicer list --json
./autonicer list --all --json
./autonicer history
./autonicer config
./autonicer memory --json
./autonicer memory-candidates --json
./autonicer pager-demo --algorithm ALGORITHM --frames N --reference LIST
./autonicer classify PID normal|background|critical
./autonicer protect PID
./autonicer unprotect PID
./autonicer restore PID
./autonicer resume PID
./autonicer monitor
./autonicer monitor --dry-run
```

Examples:

```sh
./autonicer sample
./autonicer memory --json
./autonicer list --all --json
./autonicer pager-demo --algorithm lru --frames 3 --reference '1,2,3,1,4,2'
```

`list` shows persistent registered processes. `list --all --json` shows live application processes owned by the current user. Common system/session services such as `systemd`, `sd-pam`, D-Bus, PipeWire, portal, GVFS, and tracker services are hidden from this discovery view to keep the desktop list useful; explicitly registered entries remain visible. Unregistered entries are informational and remain under the normal safety policy until explicitly classified. JSON fields are designed for the Tauri bridge and scripting.

## Safe CPU Demo

Start the controlled workload:

```sh
./demo/cpu_hog
```

In another terminal, replace `PID` with the printed process ID:

```sh
./autonicer classify PID background
./autonicer list --json
./autonicer monitor --dry-run
./autonicer history
```

Dry-run reports decisions without changing priority. For a real controlled test, stop dry-run and use `./autonicer monitor`. Stop the workload with `Ctrl-C`. Do not classify unrelated system processes.

## Safe Memory Demo

```sh
./demo/memory_hog 256
./autonicer memory --json
./autonicer memory-candidates --json
```

The argument is the approximate allocation size in MiB. Stop it with `Ctrl-C`. The workload helps demonstrate RSS, swap, and page-fault telemetry; it does not alter unrelated processes.

## Safety Model

Only explicitly classified `BACKGROUND` processes can be automatically reniced. `NORMAL`, `CRITICAL`, and `PROTECTED` processes are excluded. The core also excludes AutoNicer itself, PID 1, zombies, vanished processes, processes owned by another user, and stale registrations whose `/proc` starttime changed.

Before every action, the core validates PID identity, starttime, UID, process state, classification, and protection. High CPU without an eligible background process produces no action. Priority changes use `getpriority()` and `setpriority()`; higher nice values reduce CPU scheduling preference. Automatic pause is disabled and no process is killed. `resume` only continues a pause recorded as performed by AutoNicer.

State is stored locally under `~/.local/state/autonicer/`:

- `registry`: registered process identity and control state
- `history`: controller action log
- `config`: active policy values

## Configuration

Defaults are documented in `autonicer.conf.example`. The editable keys are:

```text
sample_interval
high_threshold
critical_threshold
high_samples_required
nice_step
max_nice
cooldown_seconds
allow_auto_pause
memory_high_available_percent
memory_critical_available_percent
memory_samples_required
```

The Policies page reads and saves these values through the C core. The Rust bridge accepts only these keys and never passes arbitrary commands to a shell.

## Desktop Pages

The application architecture is:

```text
React + TypeScript -> Tauri Rust bridge -> approved AutoNicer C executable -> Linux /proc
```

- **Overview**: live CPU percentage, controller state, process counts, protection count, CPU history, and recent audit entries.
- **Processes**: live user-owned application process data including PID, name, CPU, RSS, swap, state, classification, and protection. Common desktop/system session services are filtered from discovery. Controls classify, protect, restore, and resume through the core.
- **Memory**: available/total RAM, memory pressure state, swap, page-fault rates, swap rates, and sample timestamp.
- **Paging Lab**: user-space FIFO, LRU, and Clock simulation with configurable frames and page references. It never changes Linux memory mappings.
- **Activity**: controller history plus current CPU, load state, and available-memory telemetry, including before any action has been logged.
- **Policies**: core configuration editor with validation through the Rust bridge.
- **Settings**: Tauri-to-core connection status.

The Paging Lab controls wrap for narrow windows and mobile-sized views. If a window was already open while code changed, stop it and relaunch with `./run-desktop.sh`.

## Telemetry Sources

- CPU: `/proc/stat`
- Process CPU, state, identity, RSS, swap, and faults: `/proc/PID/stat` and `/proc/PID/status`
- Memory totals and availability: `/proc/meminfo`
- VM fault and swap counters: `/proc/vmstat`
- Optional memory pressure: `/proc/pressure/memory`

CPU percentages use deltas between `/proc/stat` samples. Process fault rates use process counter deltas and the configured sample interval. `/proc/loadavg` is not treated as a CPU percentage.

## Troubleshooting

**The desktop says the core is unavailable**

Run `make autonicer`, then restart with `./run-desktop.sh`. Confirm that `cargo` is available with `cargo --version`.

**The browser page has no live data**

Use `./run-desktop.sh`. A plain Vite browser session cannot invoke Tauri commands.

**Processes shows no registered entries**

The Processes page now displays live user-owned processes. To enable automatic control for one, classify it explicitly, for example `./autonicer classify PID background`.

**Activity has no actions**

That is normal before the monitor changes a process. Current telemetry remains visible; run the controlled CPU demo in dry-run mode to generate audit output.

**A priority restore fails**

Linux permissions or resource limits may prevent an unprivileged user from decreasing niceness. The failure is reported rather than hidden.

## Project Layout

```text
include/                 C public headers
src/                     controller, telemetry, registry, logger, pager
demo/                    controlled CPU and memory workloads
tests/                   unit and integration tests
desktop/src/             React pages, types, and Tauri IPC wrapper
desktop/src-tauri/       Rust Tauri bridge and packaging
docs/                    memory and paging design notes
run-desktop.sh           one-command Tauri development launcher
```

## Scope

The monitor runs in the foreground and state is local to one user. Kernel modules, cgroups, process killing, arbitrary command execution, web services, and kernel page-table manipulation are intentionally out of scope.
