# Architecture

```text
Terminal CLI
    |
    v
System Monitor (/proc/stat)
    |
    v
Process Registry and Classification
    |
    v
Decision Controller
    |
    v
Linux Process Control
getpriority/setpriority, SIGSTOP, SIGCONT
```

`main.c` parses commands and coordinates persistence. `monitor.c` samples aggregate CPU counters. `process.c` reads process identity and CPU counters. `registry.c` stores explicit classifications and verifies stale PIDs. `controller.c` applies the sustained-load, candidate-filtering, renice-first policy. `logger.c` records meaningful actions. `config.c` loads simple key/value settings.
