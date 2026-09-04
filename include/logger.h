#ifndef LOGGER_H
#define LOGGER_H
#include "autonicer.h"
int logger_write(const char *path, double cpu, pid_t pid, const char *name, const char *action, int oldv, int newv, int ok, const char *reason);
int logger_print(const char *path);
#endif
