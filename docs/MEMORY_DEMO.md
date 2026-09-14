# Memory and Paging Demo

First build and test:

```sh
make
make test
```

Show live Linux telemetry:

```sh
./autonicer memory
./autonicer memory --json
```

For resident-memory process data, use the controlled workload:

```sh
./demo/memory_hog 256
./autonicer classify PID background
./autonicer list --json
./autonicer memory-candidates --json
```

Stop only the controlled workload with `kill PID` after the demonstration.

Run the same reference string with each replacement algorithm:

```sh
./autonicer pager-demo --algorithm fifo --frames 3 --reference '1,2,3,1W,4,2'
./autonicer pager-demo --algorithm lru --frames 3 --reference '1,2,3,1W,4,2'
./autonicer pager-demo --algorithm clock --frames 3 --reference '1,2,3,1W,4,2'
```

Open the desktop application with `npm run tauri:dev`, then use `Memory` for live kernel telemetry and `Paging Lab` for the simulator. The UI labels these sources separately.
