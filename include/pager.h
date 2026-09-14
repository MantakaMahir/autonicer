#ifndef PAGER_H
#define PAGER_H
#include <stdio.h>
int pager_run(const char *algorithm, int frame_count, const char *reference,
              FILE *out);
#endif
