#include "registry.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *class_name(ProcessClass c) {
  return c == PROCESS_BACKGROUND ? "BACKGROUND"
         : c == PROCESS_CRITICAL ? "CRITICAL"
                                 : "NORMAL";
}
ManagedProcess *registry_find(ManagedProcess *a, size_t n, pid_t pid) {
  for (size_t i = 0; i < n; i++)
    if (a[i].info.pid == pid)
      return &a[i];
  return NULL;
}
int registry_load(ManagedProcess **out, size_t *n, const char *path) {
  FILE *f = fopen(path, "r");
  char line[512];
  *out = NULL;
  *n = 0;
  if (!f)
    return 0;
  while (fgets(line, sizeof line, f)) {
    ManagedProcess m;
    int cls, prot, orig, last, changed, paused;
    unsigned uid;
    unsigned long long st;
    if (sscanf(line, "%d %llu %u %d %d %d %d %d %d", &m.info.pid, &st, &uid,
               &cls, &prot, &orig, &last, &changed, &paused) == 9) {
      ProcessInfo p;
      if (process_read(m.info.pid, &p) == 0 && p.starttime == st &&
          p.uid == uid) {
        memset(&m, 0, sizeof m);
        m.info = p;
        m.classification = (ProcessClass)cls;
        m.protected_flag = prot;
        m.original_nice = orig;
        m.last_nice = last;
        m.priority_changed = changed;
        m.paused_by_autonicer = paused;
        *out = realloc(*out, (*n + 1) * sizeof m);
        (*out)[(*n)++] = m;
      }
    }
  }
  fclose(f);
  return 0;
}
int registry_save(const ManagedProcess *a, size_t n, const char *path) {
  FILE *f = fopen(path, "w");
  if (!f)
    return -1;
  for (size_t i = 0; i < n; i++)
    fprintf(f, "%d %llu %u %d %d %d %d %d %d\n", a[i].info.pid,
            a[i].info.starttime, a[i].info.uid, a[i].classification,
            a[i].protected_flag, a[i].original_nice, a[i].last_nice,
            a[i].priority_changed, a[i].paused_by_autonicer);
  fclose(f);
  return 0;
}
int registry_classify(ManagedProcess **a, size_t *n, pid_t pid, ProcessClass c,
                      uid_t uid) {
  ProcessInfo p;
  if (process_read(pid, &p) || p.uid != uid)
    return -1;
  ManagedProcess *m = registry_find(*a, *n, pid);
  if (!m) {
    *a = realloc(*a, (*n + 1) * sizeof **a);
    m = &(*a)[(*n)++];
    memset(m, 0, sizeof *m);
    m->info = p;
    m->original_nice = p.nice_value;
    m->last_nice = p.nice_value;
  }
  m->info = p;
  m->classification = c;
  return 0;
}
int registry_protect(ManagedProcess *a, size_t n, pid_t pid, int v) {
  ManagedProcess *m = registry_find(a, n, pid);
  if (!m)
    return -1;
  m->protected_flag = v;
  return 0;
}
void registry_cleanup(ManagedProcess *a, size_t *n) {
  for (size_t i = 0; i < *n;) {
    ProcessInfo p;
    if (process_read(a[i].info.pid, &p) || p.starttime != a[i].info.starttime) {
      a[i] = a[--*n];
    } else
      i++;
  }
}
