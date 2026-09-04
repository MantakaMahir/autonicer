# Algorithm

```text
sample CPU twice
if parsing fails: take no action
update sustained high counter
if high is not sustained or cooldown is active: continue
filter registered BACKGROUND processes
exclude protected, critical, normal, foreign, zombie, stale, and inaccessible processes
select highest recent CPU candidate
if none: report no eligible background process
revalidate identity
increase nice value up to configured cap
enter cooldown
sample again
if load improved: stop escalation
if still critical: pause is only a last-resort, confirmation-gated operation
```

The algorithm intentionally investigates before acting. Niceness is less disruptive than suspension, and suspension is less destructive than termination. AutoNicer never sends `SIGKILL` or automatically chooses a process merely because it consumes CPU.
