# Memory Telemetry

AutoNicer observes Linux memory pressure without modifying page tables. The core reads `MemTotal`, `MemAvailable`, swap counters, page-fault counters, and optional memory PSI from `/proc`. Rates are calculated from cumulative counter deltas over a sampling interval.

Memory state is independent from CPU state:

- `NORMAL`: available memory is above the configured high threshold.
- `HIGH`: available memory is below the high threshold.
- `CRITICAL`: available memory is below the critical threshold.

Process memory uses `VmRSS` for resident physical memory and `VmSwap` for swapped pages. `VmSize` is intentionally not treated as physical usage. Process identity and ownership checks remain the same as CPU actions.

Commands:

```sh
./autonicer memory
./autonicer memory --json
./autonicer list --json
./autonicer memory-candidates --json
```
