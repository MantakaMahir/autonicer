# `src` Folder Guide

This document explains every source file in the `src` folder of AutoNicer. The `.c` files contain the program's implementation. The `.o` files are compiled object files generated from the `.c` files; they are build outputs and are not separate program features.

## How the source files work together

```text
main.c
  |
  +--> config.c       Loads policy settings
  +--> monitor.c      Measures total CPU usage
  +--> memory.c       Measures memory and swap pressure
  +--> process.c      Reads and validates individual processes
  +--> registry.c     Stores classifications and protection state
  +--> controller.c   Decides whether to renice or pause a process
  +--> logger.c       Records controller actions
  +--> pager.c        Runs the educational paging simulation
```

The main monitoring path is:

```text
Linux /proc files -> monitor/process/memory modules -> controller -> nice or SIGSTOP
```

## `src/main.c`

- **What it does:** This is the program entry point and command-line controller.
- **Why we use it:** Every command starts here, so the program needs one place to initialize state and choose what operation to perform.
- **Main responsibilities:**
  - Defines `main`, which receives command-line arguments.
  - Creates the local state directory under `~/.local/state/autonicer`.
  - Loads configuration using `config.c`.
  - Loads and saves registered process information using `registry.c`.
  - Handles commands such as `sample`, `monitor`, `memory`, `list`, `classify`, `protect`, `restore`, `resume`, `kill`, `history`, and `config`.
  - Provides JSON output for the desktop application and scripts.
  - Calls the controller when the user runs the monitoring command.
  - Validates PIDs through `parse_pid` before using them.
  - Provides a guarded manual `kill` command that sends `SIGTERM` only to a
    registered, unprotected `BACKGROUND` process owned by the current user.
  - Rejects AutoNicer, PID 1, stale, foreign, zombie, protected, and
    non-background processes before a manual kill, then records successful
    termination requests through `logger.c`.
- **Simple explanation:** `main.c` is the coordinator. It does not perform every calculation itself; it calls the correct module for each command.

## `src/config.c`

- **What it does:** Loads, stores, validates, and prints AutoNicer policy settings.
- **Why we use it:** The monitoring behavior should be adjustable without changing the C source code.
- **Main responsibilities:**
  - `config_defaults` sets safe default values.
  - `config_load` reads `key=value` settings from the configuration file.
  - Ignores blank lines and lines beginning with `#`.
  - Converts numeric text with `atoi` and supports boolean values such as `true` and `1` for automatic pausing.
  - Checks important limits such as positive sample intervals, thresholds up to 100 percent, and a maximum nice value no greater than 19.
  - `config_print` displays the active settings.
- **Simple explanation:** `config.c` is the settings manager. It tells the controller how sensitive and how aggressive it is allowed to be.

## `src/controller.c`

- **What it does:** Implements the main decision-making policy for CPU pressure.
- **Why we use it:** Measurements alone do not solve the problem; this file decides when an action is justified and which process can safely be changed.
- **Main responsibilities:**
  - `controller_monitor` repeatedly samples CPU usage and tracks sustained high load.
  - Classifies the system as normal, high, or critical using configured thresholds.
  - Waits for the configured number of high samples before acting, which prevents a short CPU spike from causing a change.
  - `controller_best` selects the highest-CPU eligible background process.
  - Excludes AutoNicer itself, PID 1, protected processes, non-background processes, foreign processes, zombies, stale PIDs, and inaccessible processes.
  - `do_renice` increases a process's nice value up to the configured maximum.
  - Uses dry-run mode to report what would happen without changing the process.
  - Applies a cooldown so the same process is not modified too frequently.
  - `consider_pause` treats pausing as a last resort and requires permission when automatic pausing is enabled and the program is interactive.
  - Uses `SIGSTOP` to pause and records that AutoNicer performed the pause.
- **Simple explanation:** `controller.c` is the brain of AutoNicer. It converts monitoring information into safe process-management actions.

## `src/monitor.c`

- **What it does:** Measures total CPU utilization for the whole system.
- **Why we use it:** AutoNicer must know whether the computer is under CPU pressure before it changes a process priority.
- **Main responsibilities:**
  - `monitor_read` reads CPU counters from `/proc/stat`.
  - Adds user, system, idle, I/O wait, interrupt, steal, and guest CPU counters to calculate total time.
  - Stores idle time separately.
  - `monitor_sample` compares two readings taken at different times.
  - Calculates CPU percentage from the difference between the readings.
  - Rejects invalid or decreasing counter values rather than making a decision from bad data.
- **Simple explanation:** `monitor.c` takes two CPU snapshots and calculates how busy the system was between them.

## `src/process.c`

- **What it does:** Reads information about individual Linux processes and validates their identity.
- **Why we use it:** The controller needs accurate process data and must avoid changing the wrong process.
- **Main responsibilities:**
  - `process_read` reads process information from `/proc/PID/stat` and `/proc/PID/status`.
  - Collects the PID, process name, state, user ID, CPU time, nice value, start time, RSS memory, swap, and page faults.
  - `process_update_cpu` calculates recent CPU percentage and page-fault rates from counter differences.
  - `process_discover_owned` finds processes belonging to the current user for the process list.
  - `process_is_owned` checks process ownership.
  - `process_identity_valid` compares the current process start time and user ID with the stored values.
  - Rejects zombies and detects PID reuse. This matters because Linux can reuse a PID after an old process exits.
  - `process_is_system_service` identifies common system or desktop services that should normally be hidden from the user-facing process list.
- **Simple explanation:** `process.c` is the process information and safety layer. It tells the project what each process is doing and whether it is safe to consider.

## `src/memory.c`

- **What it does:** Measures RAM, swap, page faults, and memory pressure.
- **Why we use it:** A system can be overloaded because of low memory even when CPU usage is not the main problem.
- **Main responsibilities:**
  - Reads total and available RAM from `/proc/meminfo`.
  - Reads swap totals and free swap from `/proc/meminfo`.
  - Reads page-fault and swap activity counters from `/proc/vmstat`.
  - Optionally reads Pressure Stall Information from `/proc/pressure/memory`.
  - `memory_delta` compares two samples to calculate minor faults per second, major faults per second, and swap activity per second.
  - Classifies memory as normal, high, or critical according to configured available-memory percentages.
  - `memory_state_name` converts the state enum to readable text.
- **Simple explanation:** `memory.c` checks whether the system is running short of memory and reports the amount of memory activity.

## `src/registry.c`

- **What it does:** Manages the list of processes explicitly known to AutoNicer.
- **Why we use it:** The program should not automatically control every process. A registry records which processes the user classified and protected.
- **Main responsibilities:**
  - Stores process classification as normal, background, or critical.
  - `registry_load` reads saved process records from the local registry file.
  - Verifies the saved PID, start time, and user ID before restoring an entry.
  - `registry_save` persists classifications, protection flags, priority state, and pause state.
  - `registry_classify` adds a process or changes its classification.
  - `registry_protect` marks or unmarks a process as protected.
  - `registry_find` searches for a registered PID.
  - `registry_cleanup` removes entries for processes that disappeared or whose PID was reused.
- **Simple explanation:** `registry.c` is the project memory for user decisions. It remembers which processes are background tasks and which must be protected.

## `src/logger.c`

- **What it does:** Records controller actions in the history file.
- **Why we use it:** System-management actions should be auditable. The user should be able to see what AutoNicer attempted and whether it succeeded.
- **Main responsibilities:**
  - `logger_write` appends timestamped action records.
  - Records CPU level, PID, process name, action, old value, new value, success or failure, and an error reason when needed.
  - `logger_print` displays the saved history.
  - Handles a missing history file as an empty history rather than treating it as a fatal error.
- **Simple explanation:** `logger.c` is the audit trail. It explains what the controller did.

## `src/pager.c`

- **What it does:** Simulates page replacement algorithms in user space.
- **Why we use it:** It provides an operating-systems learning demonstration for virtual memory and page replacement without changing real Linux memory mappings.
- **Main responsibilities:**
  - Represents physical-memory frames with page number, dirty bit, reference bit, and last-used time.
  - Supports FIFO, LRU, and Clock replacement algorithms.
  - Parses a page-reference sequence such as `1,2,3,1,4`.
  - Counts hits, page faults, evictions, and write-backs.
  - Produces structured JSON describing every simulation step.
  - `choose_victim` selects the frame to replace according to the chosen algorithm.
- **Simple explanation:** `pager.c` is an educational paging laboratory. It demonstrates operating-system page replacement separately from the real process monitor.

## The compiled `.o` files

The `src` folder may also contain files such as `main.o`, `monitor.o`, and `controller.o`.

- These are object files produced by the compiler from the corresponding `.c` files.
- They contain machine-level compiled code that the linker uses to build the `autonicer` executable.
- They are not files that the developer normally explains as separate features.
- The source files ending in `.c` are the files containing the readable project logic.

## Recent small code changes

- **Registered process CPU sampling:** The `list` command now takes an initial CPU-time snapshot, waits for the configured `sample_interval`, and then calls `process_update_cpu` for registered processes. This allows the list output to show a recent CPU percentage instead of only displaying process information.
- **Guarded manual process termination:** `main.c` now supports `autonicer kill PID`. The command only sends `SIGTERM` when the process is registered, classified as `BACKGROUND`, not protected, not AutoNicer itself, not PID 1, owned by the current user, not a zombie, and still matches its saved process identity. Otherwise, termination is refused.
- **Audit logging for manual termination:** A successful manual kill request is recorded by `logger_write` with the `KILL` action, so it appears in the project history.
- **Why these changes matter:** CPU sampling improves the accuracy of process information, while the guarded kill command gives the user manual control without allowing arbitrary or unsafe process termination.

## Short teacher explanation

> The `src` folder contains the implementation of AutoNicer. `main.c` coordinates commands and modules. `monitor.c` reads total CPU usage, `process.c` reads and validates individual processes, and `memory.c` measures RAM and swap pressure. `registry.c` remembers classifications and protected processes. `controller.c` makes safe decisions and changes process priority with Linux `nice` values or, as a last resort, pauses a process. `config.c` loads policies, `logger.c` records actions, and `pager.c` provides a separate simulation of FIFO, LRU, and Clock page replacement. Together, these files form the C core of the project.
