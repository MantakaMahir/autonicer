# Graph Report - autonicer  (2026-09-15)

## Corpus Check
- Corpus is ~10,114 words - fits in a single context window. You may not need a graph.

## Summary
- 266 nodes · 410 edges · 26 communities (14 shown, 7 thin omitted)
- Extraction: 90% EXTRACTED · 10% INFERRED · 0% AMBIGUOUS · INFERRED: 41 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- Desktop Tauri Bridge
- Core Configuration
- React Desktop UI
- TypeScript Compiler Settings
- Process Controller
- Desktop Package Setup
- Tauri Application Config
- C Header Interfaces
- Project Documentation
- Frontend Development Tools
- Demo Scenarios
- Tauri Permissions
- Architecture Documentation
- Paging Simulation
- HTML Application Entry
- Sidecar Build Script
- Desktop Launch Script
- Test Runner Script
- PNG App Icon
- SVG App Icon
- Desktop App Metadata

## God Nodes (most connected - your core abstractions)
1. `main()` - 27 edges
2. `run_core_async()` - 15 edges
3. `compilerOptions` - 15 edges
4. `controller_monitor()` - 11 edges
5. `process_read()` - 10 edges
6. `scenarios.sh script` - 8 edges
7. `scripts` - 8 edges
8. `MonitorProcess` - 8 edges
9. `registry_classify()` - 8 edges
10. `core_path()` - 7 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `process_read()`  [INFERRED]
  tests/test_autonicer.c → src/process.c
- `main()` --calls--> `logger_print()`  [INFERRED]
  src/main.c → src/logger.c
- `main()` --calls--> `process_is_system_service()`  [INFERRED]
  src/main.c → src/process.c
- `main()` --calls--> `memory_read()`  [INFERRED]
  tests/test_autonicer.c → src/memory.c
- `main()` --calls--> `monitor_read()`  [INFERRED]
  tests/test_autonicer.c → src/monitor.c

## Import Cycles
- None detected.

## Communities (26 total, 7 thin omitted)

### Community 0 - "Desktop Tauri Bridge"
Cohesion: 0.18
Nodes (28): AppHandle, Child, core_path(), get_config(), get_history(), get_memory_candidates(), get_memory_status(), get_processes() (+20 more)

### Community 1 - "Core Configuration"
Cohesion: 0.13
Nodes (28): MemoryState, ProcessClass, AutoNicerConfig, config_defaults(), config_load(), config_print(), AutoNicerConfig, MemorySample (+20 more)

### Community 2 - "React Desktop UI"
Cohesion: 0.12
Nodes (13): call(), core, desktopAvailable(), isDesktopShell, pages, Action, Classification, LoadState (+5 more)

### Community 3 - "TypeScript Compiler Settings"
Cohesion: 0.10
Nodes (20): compilerOptions, allowJs, allowSyntheticDefaultImports, esModuleInterop, isolatedModules, jsx, lib, module (+12 more)

### Community 4 - "Process Controller"
Cohesion: 0.17
Nodes (18): AutoNicerConfig, ManagedProcess, pid_t, uid_t, consider_pause(), controller_best(), controller_monitor(), do_renice() (+10 more)

### Community 5 - "Desktop Package Setup"
Cohesion: 0.10
Nodes (19): dependencies, react, react-dom, @tauri-apps/api, name, private, scripts, build (+11 more)

### Community 6 - "Tauri Application Config"
Cohesion: 0.10
Nodes (19): app, security, windows, build, beforeDevCommand, devUrl, frontendDist, bundle (+11 more)

### Community 7 - "C Header Interfaces"
Cohesion: 0.13
Nodes (8): MemorySample, memory_delta(), memory_read(), read_value(), monitor_read(), monitor_sample(), SystemSample, main()

### Community 8 - "Project Documentation"
Cohesion: 0.16
Nodes (15): AutoNicer, C17 controller, Clock replacement, Demonstration And Scenario Testing, Memory and Paging Demo, Memory Telemetry, Memory telemetry, Paging Lab (+7 more)

### Community 9 - "Frontend Development Tools"
Cohesion: 0.15
Nodes (13): devDependencies, @tauri-apps/cli, @types/react, @types/react-dom, typescript, vite, @vitejs/plugin-react, @tauri-apps/cli (+5 more)

### Community 10 - "Demo Scenarios"
Cohesion: 0.38
Nodes (11): is_demo_process(), record(), require_binaries(), scenarios.sh script, start_all(), start_cpu(), start_memory(), start_workload() (+3 more)

### Community 11 - "Tauri Permissions"
Cohesion: 0.25
Nodes (7): description, identifier, permissions, $schema, windows, core:default, main

### Community 12 - "Architecture Documentation"
Cohesion: 0.50
Nodes (5): Controller algorithm, Controller Algorithm, Architecture, Linux process control, Process registry and classification

### Community 13 - "Paging Simulation"
Cohesion: 0.50
Nodes (4): FILE, Frame, choose_victim(), pager_run()

## Knowledge Gaps
- **76 isolated node(s):** `name`, `private`, `version`, `type`, `dev` (+71 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 120 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **7 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `main()` connect `Core Configuration` to `Process Controller`, `C Header Interfaces`?**
  _High betweenness centrality (0.043) - this node is a cross-community bridge._
- **Are the 21 inferred relationships involving `main()` (e.g. with `config_defaults()` and `config_load()`) actually correct?**
  _`main()` has 21 INFERRED edges - model-reasoned connections that need verification._
- **What connects `name`, `private`, `version` to the rest of the system?**
  _76 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Core Configuration` be split into smaller, more focused modules?**
  _Cohesion score 0.12903225806451613 - nodes in this community are weakly interconnected._
- **Should `React Desktop UI` be split into smaller, more focused modules?**
  _Cohesion score 0.11666666666666667 - nodes in this community are weakly interconnected._
- **Should `TypeScript Compiler Settings` be split into smaller, more focused modules?**
  _Cohesion score 0.09523809523809523 - nodes in this community are weakly interconnected._
- **Should `Desktop Package Setup` be split into smaller, more focused modules?**
  _Cohesion score 0.1 - nodes in this community are weakly interconnected._