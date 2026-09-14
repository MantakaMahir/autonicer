# Demonstration And Scenario Testing

Build the project:

```sh
make
```

The recommended scenario launcher starts only the project workloads, records their PIDs, and refuses to stop a PID unless `/proc/PID/comm` still matches the expected demo binary.

## Scenario Launcher

Build the project and inspect the available scenarios:

```sh
make
./demo/scenarios.sh --help
```

Start one CPU candidate:

```sh
./demo/scenarios.sh start-cpu
./demo/scenarios.sh status
./autonicer monitor --dry-run
```

Start the complete mixed scenario:

```sh
./demo/scenarios.sh start-all
./demo/scenarios.sh status
./autonicer monitor --dry-run
```

The mixed scenario contains one `BACKGROUND` CPU candidate, one `CRITICAL` protected CPU process, and one 256 MiB memory workload. Only the background candidate is eligible for automatic renicing.

Observe memory telemetry separately:

```sh
./autonicer memory --json
./autonicer memory-candidates --json
```

Always clean up with the launcher:

```sh
./demo/scenarios.sh stop
```

`cleanup` is an alias for `stop`. The launcher stores logs and tracked PIDs under `~/.local/state/autonicer/` and restores tracked priorities before stopping workloads.

## Manual CPU Scenario

For manual control, start two controlled workloads in separate terminals and note their PIDs:

```sh
./demo/cpu_hog
./demo/cpu_hog
```

Register one as `BACKGROUND` and protect the other:

```sh
./autonicer classify BACKGROUND_PID background
./autonicer classify IMPORTANT_PID critical
./autonicer protect IMPORTANT_PID
./autonicer list
```

Run a safe preview first:

```sh
./autonicer monitor --dry-run
```

This selects only the registered background process and never changes it. For a real controlled test, run `./autonicer monitor`; after sustained high utilization it raises the background process's nice value. Verify with:

```sh
ps -o pid,stat,ni,comm -p BACKGROUND_PID,IMPORTANT_PID
./autonicer history
```

Restore and terminate only the demo workloads:

```sh
./autonicer restore BACKGROUND_PID
kill BACKGROUND_PID IMPORTANT_PID
```

The no-action safety case is demonstrated by starting workloads without classifying either one. AutoNicer reports high CPU but no eligible background process and modifies neither workload.
