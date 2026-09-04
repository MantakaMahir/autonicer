# Demonstration

Build the project:

```sh
make
```

Start two controlled workloads in separate terminals and note their PIDs:

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
