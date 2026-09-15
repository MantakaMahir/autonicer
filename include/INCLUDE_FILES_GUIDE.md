# `include` Folder Guide

The `include` folder contains the header files shared by AutoNicer's C source
files. The headers define common data structures and declare the functions
implemented in `src/`.

## How the headers work together

```text
autonicer.h
  |
  +--> config.h       Configuration settings
  +--> monitor.h      System CPU samples
  +--> memory.h       Memory pressure samples
  +--> process.h      Process information and validation
  +--> registry.h     Saved classifications and protection
  +--> controller.h   Monitoring decisions and actions
  +--> logger.h       Action history

pager.h  -->  Independent paging simulation interface
```

`src/main.c` includes all of the module headers and coordinates the command
line application, including the guarded manual `kill` command. Most other
headers include `autonicer.h` so they can share the same process,
configuration, and system-sample types.

## `include/autonicer.h`

- **Purpose:** Defines the shared data model and small common utility APIs.
- **Main contents:** Process classes, CPU load states, `ProcessInfo`,
  `ManagedProcess`, `SystemSample`, and `AutoNicerConfig`.
- **Links:** Included by the configuration, monitoring, memory, process,
  registry, controller, and logger headers. Its declarations are implemented
  mainly in `src/main.c` and used throughout the C modules.

## `include/config.h`

- **Purpose:** Declares the configuration loading and display functions.
- **Main functions:** `config_defaults`, `config_load`, and `config_print`.
- **Links:** Includes `autonicer.h` for `AutoNicerConfig`; `src/config.c`
  implements the functions, and `src/main.c` uses them before other modules
  run.

## `include/monitor.h`

- **Purpose:** Declares the system-wide CPU sampling interface.
- **Main functions:** `monitor_read` reads a snapshot, while `monitor_sample`
  compares snapshots and calculates utilization.
- **Links:** Includes `autonicer.h` for `SystemSample`; `src/monitor.c`
  reads `/proc/stat`, and `src/main.c` and `src/controller.c` consume the
  samples.

## `include/memory.h`

- **Purpose:** Declares memory, swap, page-fault, and pressure telemetry types
  and functions.
- **Main contents:** `MemoryState`, `MemorySample`, `memory_read`,
  `memory_delta`, and `memory_state_name`.
- **Links:** Includes `autonicer.h` for shared time types; `src/memory.c`
  reads Linux memory counters, while `src/main.c` combines its results with
  configuration thresholds.

## `include/process.h`

- **Purpose:** Declares process discovery, reading, ownership, identity, and
  CPU-rate functions.
- **Main functions:** `process_read`, `process_discover_owned`,
  `process_identity_valid`, and `process_update_cpu`.
- **Links:** Includes `autonicer.h` for `ProcessInfo` and `ManagedProcess`;
  `src/process.c` reads `/proc/PID`, `src/registry.c` validates saved
  processes, and `src/controller.c` filters safe action candidates.

## `include/registry.h`

- **Purpose:** Declares persistence and lookup operations for registered
  processes.
- **Main functions:** Load/save registry data, find entries, classify
  processes, protect them, and remove stale entries.
- **Links:** Includes `autonicer.h` for registry records; `src/registry.c`
  implements the storage logic and uses `process.h` to validate process
  identity. `src/main.c` and `src/controller.c` use the registry state.

## `include/controller.h`

- **Purpose:** Declares AutoNicer's monitoring and process-control policy.
- **Main functions:** `controller_monitor` applies the configured policy, and
  `controller_best` selects the best eligible process.
- **Links:** Includes `autonicer.h` for configuration and process types;
  `src/controller.c` combines `monitor.h` and `process.h`, records outcomes
  through `logger.h`, and is called by `src/main.c`. The separate manual kill
  path in `src/main.c` also uses registry, process, and logger APIs for its
  eligibility checks and audit record.

## `include/logger.h`

- **Purpose:** Declares the action-history writing and printing interface.
- **Main functions:** `logger_write` records an action and `logger_print`
  displays saved history.
- **Links:** Includes `autonicer.h` for process IDs; `src/logger.c` manages
  the history file, and `src/controller.c` calls it after process actions.

## `include/pager.h`

- **Purpose:** Declares the educational page-replacement simulator interface.
- **Main function:** `pager_run` runs FIFO, LRU, or Clock against a page
  reference string and writes the result to a stream.
- **Links:** Only requires standard I/O types; `src/pager.c` implements the
  simulator and `src/main.c` invokes it for the `pager-demo` command. It is
  otherwise independent of the process-monitoring modules.

## Header and source pairs

Each module header normally has a matching implementation file:

| Header | Implementation |
| --- | --- |
| `autonicer.h` | Shared definitions; utility implementations in `src/main.c` |
| `config.h` | `src/config.c` |
| `monitor.h` | `src/monitor.c` |
| `memory.h` | `src/memory.c` |
| `process.h` | `src/process.c` |
| `registry.h` | `src/registry.c` |
| `controller.h` | `src/controller.c` |
| `logger.h` | `src/logger.c` |
| `pager.h` | `src/pager.c` |
