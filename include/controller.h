#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "autonicer.h"
int controller_monitor(AutoNicerConfig *cfg, ManagedProcess *items, size_t count, int dry_run, const char *log_path);
int controller_best(ManagedProcess *items, size_t count, uid_t uid, pid_t self, ManagedProcess **best);
#endif
