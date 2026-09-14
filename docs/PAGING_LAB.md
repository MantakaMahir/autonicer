# Paging Lab

The Paging Lab is a deterministic user-space simulation. It does not inspect or modify Linux page tables and it never sends signals to simulated pages.

Supported algorithms:

- FIFO replaces the oldest loaded frame.
- LRU replaces the least recently referenced frame.
- Clock gives referenced pages a second chance before selecting a victim.

References accept optional write markers, for example `1,2,3,1W,4`. A write makes the page dirty. Evicting a dirty page increments the write-back count.

```sh
./autonicer pager-demo --algorithm clock --frames 4 --reference '7,0,1,2,0,3,0,4'
```

Each JSON step contains the current reference, hit/fault result, selected frame, victim, write-back flag, and frame table.
