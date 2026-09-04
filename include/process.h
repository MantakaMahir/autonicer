#ifndef PROCESS_H
#define PROCESS_H
#include "autonicer.h"
int process_read(pid_t pid, ProcessInfo *p);
int process_identity_valid(const ManagedProcess *m, ProcessInfo *current);
int process_is_owned(const ProcessInfo *p, uid_t uid);
int process_update_cpu(ManagedProcess *m, const ProcessInfo *p, double interval_seconds);
#endif
