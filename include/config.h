#ifndef CONFIG_H
#define CONFIG_H
#include "autonicer.h"
void config_defaults(AutoNicerConfig *c);
int config_load(AutoNicerConfig *c, const char *path);
void config_print(const AutoNicerConfig *c);
#endif
