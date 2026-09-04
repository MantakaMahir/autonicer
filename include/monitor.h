#ifndef MONITOR_H
#define MONITOR_H
#include "autonicer.h"
int monitor_read(SystemSample *s);
int monitor_sample(SystemSample *previous, SystemSample *current);
#endif
