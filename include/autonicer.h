#ifndef AUTONICER_H
#define AUTONICER_H
#include <stddef.h>
#include <sys/types.h>
#include <time.h>

typedef enum {
  PROCESS_NORMAL,
  PROCESS_BACKGROUND,
  PROCESS_CRITICAL
} ProcessClass;
typedef enum { LOAD_NORMAL, LOAD_HIGH, LOAD_CRITICAL, LOAD_COOLDOWN } LoadState;

typedef struct {
  pid_t pid;
  char name[256];
  uid_t uid;
  char state;
  int nice_value;
  unsigned long long utime, stime, starttime;
  unsigned long long minor_faults, major_faults, rss_kb, swap_kb;
  double recent_cpu_percent;
  double minor_faults_per_second, major_faults_per_second;
  int valid;
} ProcessInfo;

typedef struct {
  ProcessInfo info;
  ProcessClass classification;
  int protected_flag;
  int original_nice;
  int last_nice;
  int priority_changed;
  int paused_by_autonicer;
  time_t last_action;
  unsigned long long last_utime, last_stime;
  int has_cpu_sample;
} ManagedProcess;

typedef struct {
  unsigned long long total, idle;
  struct timespec timestamp;
  double utilization;
} SystemSample;

typedef struct {
  int sample_interval, high_threshold, critical_threshold;
  int high_samples_required, nice_step, max_nice, cooldown_seconds;
  int allow_auto_pause;
  int memory_high_available_percent, memory_critical_available_percent,
      memory_samples_required;
} AutoNicerConfig;

const char *class_name(ProcessClass c);
int parse_pid(const char *s, pid_t *pid);
#endif
