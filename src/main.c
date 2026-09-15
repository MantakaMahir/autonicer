#include "autonicer.h"
#include "config.h"
#include "controller.h"
#include "logger.h"
#include "memory.h"
#include "monitor.h"
#include "pager.h"
#include "process.h"
#include "registry.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int parse_pid(const char *s, pid_t *pid) {
  char *end;
  long value = strtol(s, &end, 10);
  if (!*s || *end || value < 1 || value > 2147483647)
    return -1;
  *pid = (pid_t)value;
  return 0;
}
static void help(void) {
  puts("autonicer monitor [--dry-run]\nautonicer sample\nautonicer memory "
       "[--json]\nautonicer memory-candidates [--json]\nautonicer pager-demo "
       "--algorithm fifo|lru|clock --frames N --reference LIST\nautonicer "
       "list|status [--json|--all --json]|history|config\nautonicer classify PID "
       "normal|background|critical\nautonicer protect|unprotect PID\nautonicer "
       "restore|resume PID");
}
static void json_memory(const MemorySample *m) {
  printf("{\"timestamp\":%ld,\"memoryState\":\"%s\",\"totalKb\":%llu,"
         "\"availableKb\":%llu,\"availablePercent\":%.2f,\"swapTotalKb\":%llu,"
         "\"swapUsedKb\":%llu,\"minorFaultsPerSecond\":%.2f,"
         "\"majorFaultsPerSecond\":%.2f,\"swapInPerSecond\":%.2f,"
         "\"swapOutPerSecond\":%.2f,\"psi\":{\"available\":%s,\"someAvg10\":%."
         "2f,\"fullAvg10\":%.2f}}\n",
         (long)time(NULL), memory_state_name(m->state), m->total_kb,
         m->available_kb,
         m->total_kb ? 100.0 * m->available_kb / m->total_kb : 0,
         m->swap_total_kb, m->swap_total_kb - m->swap_free_kb,
         m->minor_faults_per_second, m->major_faults_per_second,
         m->swap_in_per_second, m->swap_out_per_second,
         m->psi_available ? "true" : "false", m->psi_some_avg10,
         m->psi_full_avg10);
}
static int read_memory(const AutoNicerConfig *c, MemorySample *m) {
  MemorySample before;
  if (memory_read(&before))
    return -1;
  sleep(c->sample_interval);
  if (memory_read(m) || memory_delta(&before, m, c->sample_interval,
                                     c->memory_high_available_percent,
                                     c->memory_critical_available_percent))
    return -1;
  return 0;
}
static int pager_command(int argc, char **argv) {
  const char *algorithm = "clock", *reference = "7,0,1,2,0,3,0,4";
  int frames = 4;
  for (int i = 2; i < argc; i++) {
    if (!strcmp(argv[i], "--algorithm") && i + 1 < argc)
      algorithm = argv[++i];
    else if (!strcmp(argv[i], "--frames") && i + 1 < argc)
      frames = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--reference") && i + 1 < argc)
      reference = argv[++i];
    else
      return -1;
  }
  return pager_run(algorithm, frames, reference, stdout);
}

int main(int argc, char **argv) {
  const char *home = getenv("HOME");
  if (!home)
    home = ".";
  AutoNicerConfig config;
  ManagedProcess *items = NULL;
  size_t count = 0;
  char dir[256], state[320], log[320], cfgpath[320], parent[256];
  snprintf(dir, sizeof dir, "%s/.local/state/autonicer", home);
  snprintf(state, sizeof state, "%s/registry", dir);
  snprintf(log, sizeof log, "%s/history", dir);
  snprintf(cfgpath, sizeof cfgpath, "%s/config", dir);
  snprintf(parent, sizeof parent, "%s/.local", home);
  mkdir(parent, 0700);
  snprintf(parent, sizeof parent, "%s/.local/state", home);
  mkdir(parent, 0700);
  mkdir(dir, 0700);
  config_defaults(&config);
  config_load(&config, cfgpath);
  registry_load(&items, &count, state);
  if (argc < 2 || !strcmp(argv[1], "help")) {
    help();
    free(items);
    return 0;
  }
  if (!strcmp(argv[1], "sample")) {
    SystemSample first, last;
    if (monitor_read(&first))
      return 1;
    sleep(config.sample_interval);
    if (monitor_sample(&first, &last))
      return 1;
    printf("{\"cpuPercent\":%.1f,\"loadState\":\"%s\",\"timestamp\":%ld}\n",
           last.utilization,
           last.utilization >= config.critical_threshold ? "CRITICAL"
           : last.utilization >= config.high_threshold   ? "HIGH"
                                                         : "NORMAL",
           (long)last.timestamp.tv_sec);
    free(items);
    return 0;
  }
  if (!strcmp(argv[1], "set-config")) {
    const char *keys[] = {"sample_interval=",
                          "high_threshold=",
                          "critical_threshold=",
                          "high_samples_required=",
                          "nice_step=",
                          "max_nice=",
                          "cooldown_seconds=",
                          "allow_auto_pause=",
                          "memory_high_available_percent=",
                          "memory_critical_available_percent=",
                          "memory_samples_required="};
    FILE *f = fopen(cfgpath, "w");
    if (!f || argc < 3) {
      if (f)
        fclose(f);
      free(items);
      return 1;
    }
    for (int i = 2; i < argc; i++) {
      int valid = 0;
      for (size_t k = 0; k < sizeof keys / sizeof keys[0]; k++)
        if (!strncmp(argv[i], keys[k], strlen(keys[k])))
          valid = 1;
      if (!valid) {
        fclose(f);
        free(items);
        return 1;
      }
      fprintf(f, "%s\n", argv[i]);
    }
    fclose(f);
    AutoNicerConfig check;
    config_defaults(&check);
    int ok = config_load(&check, cfgpath);
    free(items);
    if (!ok)
      return 1;
    puts("Configuration updated.");
    return 0;
  }
  if (!strcmp(argv[1], "memory")) {
    MemorySample m;
    if (read_memory(&config, &m)) {
      free(items);
      return 1;
    }
    if (argc > 2 && !strcmp(argv[2], "--json"))
      json_memory(&m);
    else {
      printf(
          "Memory: %s\nAvailable: %llu KB (%.1f%%)\nSwap used: %llu KB\nMinor "
          "faults/s: %.2f\nMajor faults/s: %.2f\nSwap in/out/s: %.2f / %.2f\n",
          memory_state_name(m.state), m.available_kb,
          m.total_kb ? 100.0 * m.available_kb / m.total_kb : 0,
          m.swap_total_kb - m.swap_free_kb, m.minor_faults_per_second,
          m.major_faults_per_second, m.swap_in_per_second,
          m.swap_out_per_second);
    }
    free(items);
    return 0;
  }
  if (!strcmp(argv[1], "memory-candidates")) {
    int json = argc > 2 && !strcmp(argv[2], "--json");
    if (json)
      puts("[");
    size_t printed = 0;
    for (size_t i = 0; i < count; i++) {
      ProcessInfo p;
      if (items[i].classification != PROCESS_BACKGROUND ||
          items[i].protected_flag ||
          process_identity_valid(&items[i], &p) == 0 ||
          !process_is_owned(&p, getuid()))
        continue;
      items[i].info = p;
      if (json)
        printf("%s{\"pid\":%d,\"name\":\"%s\",\"rssKb\":%llu,\"swapKb\":%llu,"
               "\"majorFaultsPerSecond\":%.2f}\n",
               printed ? "," : "", p.pid, p.name, p.rss_kb, p.swap_kb,
               p.major_faults_per_second);
      else
        printf("%d | %s | RSS=%llu KB | swap=%llu KB | major faults/s=%.2f\n",
               p.pid, p.name, p.rss_kb, p.swap_kb, p.major_faults_per_second);
      printed++;
    }
    if (json)
      puts("]");
    free(items);
    return 0;
  }
  if (!strcmp(argv[1], "pager-demo")) {
    int result = pager_command(argc, argv);
    free(items);
    return result ? 1 : 0;
  }
  if (!strcmp(argv[1], "monitor")) {
    int dry = argc > 2 && !strcmp(argv[2], "--dry-run");
    int result = controller_monitor(&config, items, count, dry, log);
    registry_save(items, count, state);
    free(items);
    return result ? 1 : 0;
  }
  if (!strcmp(argv[1], "list") || !strcmp(argv[1], "status")) {
    int all = argc > 2 && !strcmp(argv[2], "--all");
    int json = (argc > 2 && !strcmp(argv[2], "--json")) ||
               (all && argc > 3 && !strcmp(argv[3], "--json"));
    ManagedProcess *display = items;
    size_t display_count = count;
    if (all && (!json || process_discover_owned(&display, &display_count))) {
      free(items);
      return 1;
    }
    if (all) {
      for (size_t i = 0; i < display_count; i++) {
        display[i].last_utime = display[i].info.utime;
        display[i].last_stime = display[i].info.stime;
        display[i].has_cpu_sample = 1;
      }
      sleep(config.sample_interval);
    } else {
      for (size_t i = 0; i < display_count; i++) {
        display[i].last_utime = display[i].info.utime;
        display[i].last_stime = display[i].info.stime;
        display[i].has_cpu_sample = 1;
      }
      sleep(config.sample_interval);
    }
    if (json)
      puts("[");
    size_t printed = 0;
    for (size_t i = 0; i < display_count; i++) {
      ManagedProcess *registered =
          registry_find(items, count, display[i].info.pid);
      if (all && process_is_system_service(&display[i].info) && !registered)
        continue;
      if (all && registered) {
        display[i].classification = registered->classification;
        display[i].protected_flag = registered->protected_flag;
        display[i].priority_changed = registered->priority_changed;
        display[i].paused_by_autonicer = registered->paused_by_autonicer;
        display[i].last_nice = registered->last_nice;
      }
      ProcessInfo p;
      if (process_read(display[i].info.pid, &p) == 0) {
        process_update_cpu(&display[i], &p, config.sample_interval);
      } else {
        continue;
      }
      if (json)
        printf("%s{\"pid\":%d,\"name\":\"%s\",\"cpuPercent\":%.1f,\"nice\":%d,"
               "\"state\":\"%c\",\"rssKb\":%llu,\"swapKb\":%llu,"
               "\"minorFaultsPerSecond\":%.2f,\"majorFaultsPerSecond\":%.2f,"
               "\"classification\":\"%s\",\"protected\":%s,\"priorityChanged\":"
               "%s,\"paused\":%s}\n",
               printed++ ? "," : "", display[i].info.pid, display[i].info.name,
               display[i].info.recent_cpu_percent, display[i].info.nice_value,
               display[i].info.state, display[i].info.rss_kb,
               display[i].info.swap_kb, display[i].info.minor_faults_per_second,
               display[i].info.major_faults_per_second,
               class_name(display[i].classification),
               display[i].protected_flag ? "true" : "false",
               display[i].priority_changed ? "true" : "false",
               display[i].paused_by_autonicer ? "true" : "false");
      else {
        printf("%d | %s | nice=%d | %s | %s\n", display[i].info.pid,
               display[i].info.name, display[i].last_nice,
               class_name(display[i].classification),
               display[i].protected_flag ? "PROTECTED" : "-");
        printed++;
      }
    }
    if (json)
      puts("]");
    if (all)
      free(display);
    free(items);
    return 0;
  }
  if (!strcmp(argv[1], "history")) {
    int result = logger_print(log);
    free(items);
    return result ? 1 : 0;
  }
  if (!strcmp(argv[1], "config")) {
    config_print(&config);
    free(items);
    return 0;
  }
  pid_t pid;
  if (argc < 3 || parse_pid(argv[2], &pid)) {
    help();
    free(items);
    return 1;
  }
  int result = 0;
  if (!strcmp(argv[1], "classify") && argc >= 4) {
    ProcessClass cls;
    if (!strcmp(argv[3], "background"))
      cls = PROCESS_BACKGROUND;
    else if (!strcmp(argv[3], "critical"))
      cls = PROCESS_CRITICAL;
    else if (!strcmp(argv[3], "normal"))
      cls = PROCESS_NORMAL;
    else
      result = -1;
    if (!result)
      result = registry_classify(&items, &count, pid, cls, getuid());
  } else if (!strcmp(argv[1], "protect") || !strcmp(argv[1], "unprotect")) {
    if (!strcmp(argv[1], "protect") && !registry_find(items, count, pid))
      result = registry_classify(&items, &count, pid, PROCESS_NORMAL, getuid());
    if (!result)
      result = registry_protect(items, count, pid, !strcmp(argv[1], "protect"));
  }
  else if (!strcmp(argv[1], "restore")) {
    ManagedProcess *m = registry_find(items, count, pid);
    if (!m || !m->priority_changed)
      result = -1;
    else if (setpriority(PRIO_PROCESS, pid, m->original_nice)) {
      fprintf(stderr, "Restore failed: %s\n", strerror(errno));
      result = -1;
    } else {
      m->last_nice = m->original_nice;
      puts("Priority restored.");
    }
  } else if (!strcmp(argv[1], "resume")) {
    ManagedProcess *m = registry_find(items, count, pid);
    if (!m || !m->paused_by_autonicer || kill(pid, SIGCONT)) {
      result = -1;
      puts("Resume failed or process was not paused by AutoNicer.");
    } else {
      m->paused_by_autonicer = 0;
      puts("Process resumed.");
    }
  } else
    result = -1;
  if (result)
    fprintf(stderr, "Operation failed safely.\n");
  registry_cleanup(items, &count);
  registry_save(items, count, state);
  free(items);
  return result ? 1 : 0;
}
