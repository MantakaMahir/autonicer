# AutoNicer CLI Commands

This is the complete command reference for the `autonicer` executable. Run commands from the repository root after building with `make`.

## Build

```bash
make
```

Run the automated tests:

```bash
make test
```

## Help

```bash
./autonicer help
```

Displays the supported commands and arguments.

## CPU System Sample

```bash
./autonicer sample
```

Reads two `/proc/stat` samples and returns CPU usage, load state, and timestamp.

Example output:

```json
{"cpuPercent":21.4,"loadState":"NORMAL","timestamp":1789445702}
```

## List Registered Processes

```bash
./autonicer list
```

Shows processes stored in the AutoNicer registry.

JSON output:

```bash
./autonicer list --json
```

Example:

```json
{"pid":17100,"name":"cpu_hog","cpuPercent":100.0,"nice":0,"state":"R","rssKb":1728,"swapKb":0,"classification":"BACKGROUND","protected":false,"priorityChanged":false,"paused":false}
```

The registered list takes a live sample interval, so CPU and page-fault rates are calculated from current `/proc` data.

## List Live User Processes

```bash
./autonicer list --all --json
```

Shows live processes owned by the current user with CPU, RSS, swap, and fault rates. Common desktop/session services are filtered from discovery unless explicitly registered.

Example:

```json
{"pid":5793,"name":"firefox","cpuPercent":7.5,"nice":0,"state":"S","rssKb":819484,"swapKb":0,"minorFaultsPerSecond":1.00,"majorFaultsPerSecond":0.00,"classification":"NORMAL","protected":false,"priorityChanged":false,"paused":false}
```

## Classify A Process

```bash
./autonicer classify PID normal
./autonicer classify PID background
./autonicer classify PID critical
```

Examples:

```bash
./autonicer classify 17100 background
./autonicer classify 17105 critical
```

Only a current-user process can be registered. `BACKGROUND` is the only classification eligible for automatic CPU relief actions.

## Protect A Process

```bash
./autonicer protect PID
```

Example:

```bash
./autonicer protect 17105
```

Protecting a process excludes it from automatic controller actions. If the process is not registered, AutoNicer registers it as `NORMAL` first.

## Unprotect A Process

```bash
./autonicer unprotect PID
```

Example:

```bash
./autonicer unprotect 17105
```

Unprotecting does not automatically make a process eligible. It must also be classified as `BACKGROUND`.

## Restore Priority

```bash
./autonicer restore PID
```

Example:

```bash
./autonicer restore 17100
```

Restores the original nice value after AutoNicer has changed it. This may fail if Linux permissions prevent lowering the nice value.

## Resume A Paused Process

```bash
./autonicer resume PID
```

Example:

```bash
./autonicer resume 17100
```

Resume only works for a process that AutoNicer recorded as paused. AutoNicer does not resume arbitrary externally stopped processes.

## Manual Kill Switch

```bash
./autonicer kill PID
```

Example:

```bash
./autonicer kill 17100
```

The command sends `SIGTERM` only when all conditions are true:

- The process is registered.
- Its classification is `BACKGROUND`.
- It is not protected.
- It is owned by the current user.
- Its PID and `/proc` start time still match the registry.
- It is not a zombie.
- It is not PID 1 or AutoNicer itself.

Successful output:

```text
Process termination requested for PID 17100.
```

Rejected output:

```text
Kill refused: process is not an eligible background workload.
Operation failed safely.
```

The command never executes a shell command and never uses `SIGKILL`.

## CPU Monitor

Run the controller in the foreground:

```bash
./autonicer monitor
```

Run a preview without changing process priorities:

```bash
./autonicer monitor --dry-run
```

Stop the foreground monitor with:

```text
Ctrl+C
```

The controller waits for sustained pressure, selects only eligible `BACKGROUND` processes, and increases their nice value. The desktop app starts its own monitor automatically through Tauri.

## History

```bash
./autonicer history
```

Displays logged controller actions, including renice and kill events.

Example:

```text
2026-09-15 16:27:10 cpu=91.2 pid=17100 cpu_hog action=NICE old=0 new=2 ok=1
```

## Configuration

Show the active configuration:

```bash
./autonicer config
```

Example output:

```text
sample_interval=2
high_threshold=70
critical_threshold=90
high_samples_required=3
nice_step=2
max_nice=19
cooldown_seconds=30
allow_auto_pause=0
memory_high_available_percent=20
memory_critical_available_percent=10
memory_samples_required=3
```

Update configuration values:

```bash
./autonicer set-config sample_interval=2 high_threshold=70 critical_threshold=90
```

All accepted configuration keys:

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

The desktop Policies page uses the same C-core configuration path.

## Memory Telemetry

Human-readable output:

```bash
./autonicer memory
```

Machine-readable output:

```bash
./autonicer memory --json
```

Data sources include `/proc/meminfo`, `/proc/vmstat`, and optional `/proc/pressure/memory`.

Example fields:

```json
{
  "memoryState":"NORMAL",
  "availableKb":8777808,
  "availablePercent":56.53,
  "swapUsedKb":0,
  "minorFaultsPerSecond":1323.00,
  "majorFaultsPerSecond":0.00,
  "swapInPerSecond":0.00,
  "swapOutPerSecond":0.00,
  "psi":{"available":true,"someAvg10":0.00,"fullAvg10":0.00}
}
```

## Memory Candidates

```bash
./autonicer memory-candidates
./autonicer memory-candidates --json
```

Lists current registered `BACKGROUND`, unprotected, current-user processes with RSS, swap, and major-fault data. `NORMAL`, `CRITICAL`, protected, stale, foreign, and zombie processes are excluded.

## Paging Lab Simulation

Run the default Clock simulation:

```bash
./autonicer pager-demo
```

Select an algorithm and frame count:

```bash
./autonicer pager-demo --algorithm fifo --frames 3 --reference '1,2,3,1,4,2'
./autonicer pager-demo --algorithm lru --frames 3 --reference '1,2,3,1,4,2'
./autonicer pager-demo --algorithm clock --frames 4 --reference '7,0,1,2,0,3,0,4'
```

Use `W` to mark a write reference:

```bash
./autonicer pager-demo --algorithm fifo --frames 2 --reference '1W,2,1,3W'
```

The result includes:

- Hits
- Faults
- Evictions
- Dirty-page write-backs
- Per-reference frame state
- Selected victim frame

The Paging Lab is a user-space simulation. It does not modify Linux page tables or the memory of real processes.

## Controlled Demo Workloads

Build the workloads:

```bash
make
```

Start the complete showcase:

```bash
./demo/scenarios.sh start-all
```

View status:

```bash
./demo/scenarios.sh status
```

Stop all tracked demo processes safely:

```bash
./demo/scenarios.sh stop
```

See [`SHOWCASE.md`](SHOWCASE.md) for the complete presentation script.

## State Locations

AutoNicer stores local state under:

```text
~/.local/state/autonicer/
```

Files include:

- `registry`: registered process identities and policy state
- `history`: controller action log
- `config`: active configuration
- `demo-pids`: tracked demo workload PIDs

## Safety Reminder

AutoNicer validates process ownership, PID identity, start time, process state, classification, and protection before changes. Do not test against unrelated system processes. Use the controlled workloads in `demo/`.
