# AutoNicer

AutoNicer is a terminal Linux controller for explicitly registered background jobs. It measures actual CPU utilization, waits for sustained pressure, and raises the nice value of the safest eligible job before it considers a user-confirmed pause. It never automatically kills processes.

## Safety Policy

Only processes explicitly classified `BACKGROUND` may be changed. `NORMAL`, `CRITICAL`, and `PROTECTED` processes are excluded. AutoNicer also excludes itself, PID 1, zombies, vanished processes, processes owned by another user, and registrations whose `/proc` starttime no longer matches. High CPU alone therefore results in no action when no safe candidate exists.

## Build

```sh
make
make test
```

The build uses C17 and `-Wall -Wextra -Wpedantic`.

## Usage

```sh
./autonicer classify PID background
./autonicer protect PID
./autonicer monitor
./autonicer monitor --dry-run
./autonicer list
./autonicer history
./autonicer restore PID
./autonicer resume PID
```

The registry and history are stored under `~/.local/state/autonicer/`. Configuration is read from `~/.local/state/autonicer/config`; defaults are shown in `autonicer.conf.example`.

## How It Works

CPU utilization comes from two `/proc/stat` samples. The total contains user, nice, system, idle, iowait, IRQ, softIRQ, steal, guest, and guest-nice counters. Idle is `idle + iowait`, which is documented and conservative for this project. `/proc/loadavg` is not treated as a percentage.

Process metadata comes from `/proc/PID/stat` and `/proc/PID/status`. Recent process CPU activity uses changes in user/system CPU ticks and `sysconf(_SC_CLK_TCK)`. Before an action, PID existence, starttime, UID, state, class, and protection are checked again.

The controller waits for consecutive high readings, selects the highest recent CPU eligible background process, and uses `getpriority()`/`setpriority()` to increase its nice value. Higher nice means less favorable CPU scheduling. A configured cooldown prevents repeated actions. If pressure remains critical, pausing with `SIGSTOP` is only a last-resort policy; the current implementation reports that pause is confirmation-gated and does not perform an automatic pause. `SIGCONT` is only sent for pauses owned by AutoNicer.

Restoring a lower nice value can fail for an unprivileged user because Linux permissions/resource limits may forbid decreasing niceness. Failures are reported rather than hidden.

## Safe Demo

```sh
./demo/cpu_hog
```

In another terminal, classify the printed PID:

```sh
./autonicer classify PID background
./autonicer monitor --dry-run
```

For a real action, use `./autonicer monitor` and stop the workload with `Ctrl-C`. Use only these controlled demo processes. A protected workload can be registered and protected to demonstrate exclusion.

## Limitations and Future Work

The monitor runs in the foreground, the default pause policy is disabled, and state is local to one user. A future version could add a clearly bounded interactive pause prompt, richer test fixtures, or optional ncurses output. Kernel modules, threads, cgroups, web interfaces, and process killing are intentionally out of scope.

## Desktop UI

The `desktop/` directory contains a Tauri 2 + React + TypeScript interface. Its architecture is:

```text
React UI -> Tauri Rust bridge -> approved AutoNicer C executable -> Linux
```

The UI invokes only fixed semantic commands through Rust. The C core remains authoritative for `/proc`, classification, permissions, priority changes, and signals. The adapter adds `sample` for one-shot JSON system status and `list --json` for registered process snapshots; existing CLI behavior is preserved.

Install the native toolchain first if needed:

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
. "$HOME/.cargo/env"
```

On a Linux machine with Rust, Cargo, and Node.js installed:

```sh
make desktop-install
make desktop-build
npm run tauri dev --prefix desktop
npm run tauri build --prefix desktop
```

The production bundle uses the compiled C executable as a Tauri external binary. Desktop core functionality is Linux-only; the web assets may build elsewhere, but process controls require Linux.

## Memory and Paging Lab

The core also exposes read-only Linux memory telemetry and a safe educational paging simulator:

```sh
./autonicer memory --json
./autonicer pager-demo --algorithm clock --frames 4 --reference '7,0,1,2,0,3,0,4'
```

The controlled `demo/memory_hog` workload can be used to observe real RSS and fault data without touching unrelated processes.

Live memory values come from `/proc/meminfo`, `/proc/vmstat`, and optional memory PSI. The Paging Lab is entirely user-space and simulated; it never changes Linux page tables. See `docs/MEMORY.md`, `docs/PAGING_LAB.md`, and `docs/MEMORY_DEMO.md`.
