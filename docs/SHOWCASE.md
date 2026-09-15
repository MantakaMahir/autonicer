# AutoNicer Showcase

This guide is a copy-ready demonstration for AutoNicer. It uses only the controlled workloads in `demo/` and does not target unrelated system processes.

## 1. Build The Project

Run from the repository root:

```bash
make
make test
```

Expected result:

```text
unit tests passed
integration smoke tests passed
```

## 2. Start The Desktop App

In Terminal 1:

```bash
./run-desktop.sh
```

Leave this terminal running. The Tauri app starts the C monitor automatically and continuously refreshes Linux data.

If the app is already open, keep it open and continue with the workload setup below.

## 3. Start The Complete Demonstration

In Terminal 2:

```bash
./demo/scenarios.sh start-all
```

This starts three controlled processes:

- One `cpu_hog` classified as `BACKGROUND` and eligible for safe priority reduction.
- One `cpu_hog` classified as `CRITICAL` and protected.
- One `memory_hog` using approximately 256 MiB.

The command prints the PIDs and log locations. Save the displayed PIDs if you want to refer to them during the presentation.

## 4. Show Workload Status

In Terminal 2:

```bash
./demo/scenarios.sh status
```

Show the complete live process JSON:

```bash
./autonicer list --all --json
```

Show registered controller state:

```bash
./autonicer list --json
```

Show live memory and paging telemetry:

```bash
./autonicer memory --json
./autonicer memory-candidates --json
```

## 5. Demonstrate The Desktop Pages

Use the Tauri desktop window.

### Overview

Show:

- CPU utilization
- Controller state
- Registered process count
- Protected process count
- CPU history
- Audit trail

### Processes

Search for:

```text
cpu_hog
```

Then search by one of the displayed PIDs.

Click a process row to show its details. Demonstrate that the process table includes CPU, RSS, swap, classification, protection, and controls.

### Memory

Show:

- Memory pressure
- Available RAM
- Swap usage
- Major faults
- Minor faults
- Swap in/out
- PSI values when the Linux PSI interface is available
- Latest OS sample timestamp

### Paging Lab

Use these settings:

```text
Algorithm: Clock
Frames: 4
References: 7,0,1,2,0,3,0,4
```

Click **Run**, then use **Previous** and **Next** to demonstrate:

- Page hits
- Page faults
- Frame allocation
- Evictions
- Write-back status

Repeat with `FIFO` and `LRU` to compare the algorithms.

### Policies

Show the policy values loaded from the C core. Do not change values during the first demonstration.

### Settings

Show that the Tauri desktop bridge is connected. Browser-only Vite mode cannot access live Linux data.

## 6. Demonstrate Dry-Run Decisions

The desktop app runs the live controller automatically. For a separate terminal demonstration that does not change process priority, use the CLI in Terminal 3:

```bash
./autonicer monitor --dry-run
```

Leave it running long enough to collect several samples. It reports high CPU and explains what would happen without changing the demo process.

Stop the CLI dry-run with:

```text
Ctrl+C
```

Review the action history:

```bash
./autonicer history
```

The protected critical workload must not be selected as an automatic candidate.

## 7. Demonstrate A Real Priority Change

Only do this with the controlled `cpu_hog` workload.

First inspect the registered state:

```bash
./autonicer list --json
```

The `BACKGROUND` process is the only workload eligible for automatic renicing. The protected `CRITICAL` process must remain excluded.

Run the real controller from Terminal 3 only if you want to demonstrate an actual nice-value change:

```bash
./autonicer monitor
```

Stop it after the action appears:

```text
Ctrl+C
```

Review the result:

```bash
./autonicer list --json
./autonicer history
```

Restore the controlled process priority if it changed:

```bash
./demo/scenarios.sh stop
```

## 8. Explain The Safety Model

Use these points during the showcase:

- Only explicitly classified `BACKGROUND` processes are eligible for automatic renicing.
- `NORMAL`, `CRITICAL`, and `PROTECTED` processes are excluded.
- PID ownership and process start time are validated before actions.
- AutoNicer never kills processes.
- The Paging Lab is a user-space simulation and never modifies Linux page tables.
- The C core remains authoritative for telemetry, policy, and process actions.

## 9. Demonstrate The Manual Kill Switch

The CLI can manually terminate only a registered, unprotected `BACKGROUND` demo process. It refuses `NORMAL`, `CRITICAL`, protected, foreign, stale, zombie, PID 1, and AutoNicer processes.

Start the complete scenario:

```bash
./demo/scenarios.sh start-all
./demo/scenarios.sh status
```

Copy the PID of the `background` `cpu_hog` process from the status output, then replace `BACKGROUND_PID` below:

```bash
./autonicer kill BACKGROUND_PID
```

Expected output:

```text
Process termination requested for PID BACKGROUND_PID.
```

Verify that it exited:

```bash
./demo/scenarios.sh status
```

The protected critical process must not be killable. Copy its PID and test the safety refusal:

```bash
./autonicer kill PROTECTED_PID
```

Expected result:

```text
Kill refused: process is not an eligible background workload.
Operation failed safely.
```

The kill action uses `SIGTERM`, requires current process identity and ownership validation, and never uses `SIGKILL` or arbitrary shell execution.

## 10. Cleanup

Always stop the controlled workloads when finished:

```bash
./demo/scenarios.sh stop
```

Verify cleanup:

```bash
./demo/scenarios.sh status
```

Expected result:

```text
No tracked demo workloads.
```

Close the desktop window normally. Tauri cleans up its background monitor process when the application exits.

## Quick Copy Version

For the shortest demonstration:

```bash
make
make test
./demo/scenarios.sh start-all
./demo/scenarios.sh status
./autonicer memory --json
./autonicer list --all --json
./autonicer pager-demo --algorithm clock --frames 4 --reference '7,0,1,2,0,3,0,4'
./demo/scenarios.sh stop
```

Run the desktop app separately with:

```bash
./run-desktop.sh
```
