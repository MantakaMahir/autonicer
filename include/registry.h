#ifndef REGISTRY_H
#define REGISTRY_H
#include "autonicer.h"
int registry_load(ManagedProcess **items, size_t *count, const char *path);
int registry_save(const ManagedProcess *items, size_t count, const char *path);
ManagedProcess *registry_find(ManagedProcess *items, size_t count, pid_t pid);
int registry_classify(ManagedProcess **items, size_t *count, pid_t pid,
                      ProcessClass c, uid_t uid);
int registry_protect(ManagedProcess *items, size_t count, pid_t pid, int value);
void registry_cleanup(ManagedProcess *items, size_t *count);
#endif
