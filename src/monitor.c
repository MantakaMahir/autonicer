#include "monitor.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int monitor_read(SystemSample *s){FILE*f=fopen("/proc/stat","r");char line[512],tag[16];unsigned long long u,n,sys,idle,iowait,irq,soft,steal,guest,gn; if(!f)return -1; if(!fgets(line,sizeof line,f)||sscanf(line,"%15s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",tag,&u,&n,&sys,&idle,&iowait,&irq,&soft,&steal,&guest,&gn)<5||strcmp(tag,"cpu")){fclose(f);return -1;} s->total=u+n+sys+idle+iowait+irq+soft+steal+guest+gn; s->idle=idle+iowait; clock_gettime(CLOCK_MONOTONIC,&s->timestamp); fclose(f); return 0;}
int monitor_sample(SystemSample *previous,SystemSample *current){unsigned long long dt,di;if(monitor_read(current))return -1;if(current->total<=previous->total||current->idle<previous->idle)return -1;dt=current->total-previous->total;di=current->idle-previous->idle;if(di>dt)return -1;current->utilization=100.0*(double)(dt-di)/(double)dt;return 0;}
