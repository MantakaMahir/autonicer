#ifndef MEMORY_H
#define MEMORY_H
#include "autonicer.h"

typedef enum { MEMORY_NORMAL, MEMORY_HIGH, MEMORY_CRITICAL } MemoryState;
typedef struct {
  unsigned long long total_kb, available_kb, swap_total_kb, swap_free_kb;
  unsigned long long pgfault, pgmajfault, pswpin, pswpout;
  double minor_faults_per_second, major_faults_per_second, swap_in_per_second,
      swap_out_per_second;
  double psi_some_avg10, psi_full_avg10;
  int psi_available;
  MemoryState state;
  struct timespec timestamp;
} MemorySample;
int memory_read(MemorySample *m);
int memory_delta(const MemorySample *before, MemorySample *after,
                 double seconds, int high_available_percent,
                 int critical_available_percent);
const char *memory_state_name(MemoryState s);
#endif
