#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
int logger_write(const char*p,double cpu,pid_t pid,const char*name,const char*action,int oldv,int newv,int ok,const char*reason){FILE*f=fopen(p,"a");time_t t=time(NULL);struct tm tm;if(!f)return -1;localtime_r(&t,&tm);fprintf(f,"%04d-%02d-%02d %02d:%02d:%02d CPU=%.1f PID=%d %s %s %d->%d %s%s%s\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,cpu,pid,name,action,oldv,newv,ok?"SUCCESS":"FAIL",ok?"":" ",ok?"":reason);fclose(f);return 0;}
int logger_print(const char*p){FILE*f=fopen(p,"r");char line[512];if(!f){perror(p);return -1;}while(fgets(line,sizeof line,f))fputs(line,stdout);fclose(f);return 0;}
