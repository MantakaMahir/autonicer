#include "process.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int process_is_system_service(const ProcessInfo *p) {
  static const char *const exact[] = {
      "systemd",       "sd-pam",       "dbus-daemon", "dbus-broker",
      "pipewire",      "pipewire-pulse", "wireplumber", "pulseaudio",
      "gpg-agent",     "ssh-agent",    "gnome-keyring-daemon",
      "at-spi-bus-launcher",
  };
  static const char *const prefixes[] = {
      "systemd-", "xdg-desktop-portal", "gvfsd", "tracker-",
  };
  size_t i;
  for (i = 0; i < sizeof exact / sizeof exact[0]; i++)
    if (!strcmp(p->name, exact[i]))
      return 1;
  for (i = 0; i < sizeof prefixes / sizeof prefixes[0]; i++)
    if (!strncmp(p->name, prefixes[i], strlen(prefixes[i])))
      return 1;
  return 0;
}

int process_discover_owned(ManagedProcess **out, size_t *count) {
  DIR *dir = opendir("/proc");
  struct dirent *entry;
  ManagedProcess *items = NULL;
  size_t used = 0, capacity = 0;
  uid_t owner = getuid();
  if (!dir)
    return -1;
  while ((entry = readdir(dir))) {
    char *end;
    long value;
    ProcessInfo info;
    ManagedProcess *next;
    if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
      continue;
    errno = 0;
    value = strtol(entry->d_name, &end, 10);
    if (errno || *end || value <= 0 || value > 2147483647L ||
        process_read((pid_t)value, &info) || info.uid != owner)
      continue;
    if (used == capacity) {
      capacity = capacity ? capacity * 2 : 32;
      next = realloc(items, capacity * sizeof *items);
      if (!next) {
        free(items);
        closedir(dir);
        return -1;
      }
      items = next;
    }
    memset(&items[used], 0, sizeof items[used]);
    items[used].info = info;
    items[used].classification = PROCESS_NORMAL;
    used++;
  }
  closedir(dir);
  *out = items;
  *count = used;
  return 0;
}

int process_read(pid_t pid, ProcessInfo *p) {
  char path[64], line[4096], *close;
  unsigned long long utime, stime, start;
  long long v[16];
  FILE *f;
  int n;
  memset(p, 0, sizeof *p);
  p->pid = pid;
  snprintf(path, sizeof path, "/proc/%d/stat", pid);
  f = fopen(path, "r");
  if (!f)
    return -1;
  if (!fgets(line, sizeof line, f)) {
    fclose(f);
    return -1;
  }
  fclose(f);
  close = strrchr(line, ')');
  if (!close || close[1] != ' ')
    return -1;
  if (sscanf(line, "%d (%255[^)])", &n, p->name) != 2)
    return -1;
  char state;
  int parsed = sscanf(close + 2,
                      "%c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld "
                      "%llu %llu %lld %lld %lld %lld %lld %lld %llu",
                      &state, &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6],
                      &v[7], &v[8], &v[9], &utime, &stime, &v[10], &v[11],
                      &v[12], &v[13], &v[14], &v[15], &start);
  if (parsed < 20)
    return -1;
  p->state = state;
  p->utime = utime;
  p->stime = stime;
  p->minor_faults = (unsigned long long)v[6];
  p->major_faults = (unsigned long long)v[8];
  p->nice_value = (int)v[13];
  p->starttime = start;
  snprintf(path, sizeof path, "/proc/%d/status", pid);
  f = fopen(path, "r");
  if (f) {
    unsigned uid;
    while (fgets(line, sizeof line, f)) {
      if (sscanf(line, "Uid:\t%u", &uid) == 1)
        p->uid = (uid_t)uid;
      else if (sscanf(line, "VmRSS:\t%llu", &p->rss_kb) == 1) {
      } else if (sscanf(line, "VmSwap:\t%llu", &p->swap_kb) == 1) {
      }
    }
    fclose(f);
  } else
    return -1;
  p->valid = 1;
  return 0;
}
int process_identity_valid(const ManagedProcess *m, ProcessInfo *c) {
  return process_read(m->info.pid, c) == 0 &&
         c->starttime == m->info.starttime && c->uid == m->info.uid &&
         c->state != 'Z';
}
int process_is_owned(const ProcessInfo *p, uid_t uid) { return p->uid == uid; }
int process_update_cpu(ManagedProcess *m, const ProcessInfo *p,
                       double seconds) {
  unsigned long long now = p->utime + p->stime,
                     old = m->last_utime + m->last_stime;
  unsigned long long old_minor = m->info.minor_faults,
                     old_major = m->info.major_faults;
  double cpu = 0, minor = 0, major = 0;
  if (m->has_cpu_sample && now >= old && p->minor_faults >= old_minor &&
      p->major_faults >= old_major && seconds > 0) {
    cpu = 100.0 * (double)(now - old) / (double)sysconf(_SC_CLK_TCK) / seconds;
    minor = (p->minor_faults - old_minor) / seconds;
    major = (p->major_faults - old_major) / seconds;
  }
  m->last_utime = p->utime;
  m->last_stime = p->stime;
  m->has_cpu_sample = 1;
  m->info = *p;
  m->info.recent_cpu_percent = cpu;
  m->info.minor_faults_per_second = minor;
  m->info.major_faults_per_second = major;
  return 0;
}
