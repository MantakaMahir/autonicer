#ifndef AUTONICER_H
#define AUTONICER_H
#include <sys/types.h>
#include <time.h>
#include <stddef.h>

typedef enum { PROCESS_NORMAL, PROCESS_BACKGROUND, PROCESS_CRITICAL } ProcessClass;
typedef enum { LOAD_NORMAL, LOAD_HIGH, LOAD_CRITICAL, LOAD_COOLDOWN } LoadState;

typedef struct {
    pid_t pid;
    char name[256];
    uid_t uid;
    char state;
    int nice_value;
    unsigned long long utime, stime, starttime;
    double recent_cpu_percent;
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
} AutoNicerConfig;

const char *class_name(ProcessClass c);
int parse_pid(const char *s, pid_t *pid);
#endif
