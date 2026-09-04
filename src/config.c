#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void config_defaults(AutoNicerConfig *c) { *c=(AutoNicerConfig){2,80,90,3,5,19,8,0}; }
int config_load(AutoNicerConfig *c,const char *path) {
 FILE *f=fopen(path,"r"); char key[64], val[64], line[256]; if(!f) return 0;
 while(fgets(line,sizeof line,f)) { if(line[0]=='#'||line[0]=='\n') continue; if(sscanf(line,"%63[^=]=%63s",key,val)!=2) continue;
  int v=atoi(val); if(!strcmp(key,"sample_interval"))c->sample_interval=v; else if(!strcmp(key,"high_threshold"))c->high_threshold=v; else if(!strcmp(key,"critical_threshold"))c->critical_threshold=v; else if(!strcmp(key,"high_samples_required"))c->high_samples_required=v; else if(!strcmp(key,"nice_step"))c->nice_step=v; else if(!strcmp(key,"max_nice"))c->max_nice=v; else if(!strcmp(key,"cooldown_seconds"))c->cooldown_seconds=v; else if(!strcmp(key,"allow_auto_pause"))c->allow_auto_pause=!strcmp(val,"true")||v;
 } fclose(f); return c->sample_interval>0&&c->high_threshold>=0&&c->high_threshold<=100&&c->critical_threshold<=100&&c->high_samples_required>0&&c->nice_step>0&&c->max_nice<=19&&c->cooldown_seconds>=0;
}
void config_print(const AutoNicerConfig *c){printf("sample_interval=%d\nhigh_threshold=%d\ncritical_threshold=%d\nhigh_samples_required=%d\nnice_step=%d\nmax_nice=%d\ncooldown_seconds=%d\nallow_auto_pause=%s\n",c->sample_interval,c->high_threshold,c->critical_threshold,c->high_samples_required,c->nice_step,c->max_nice,c->cooldown_seconds,c->allow_auto_pause?"true":"false");}
