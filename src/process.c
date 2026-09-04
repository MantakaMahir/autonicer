#include "process.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int process_read(pid_t pid,ProcessInfo*p){char path[64],line[4096],*close;unsigned long long utime,stime,start;long long v[16];FILE*f;int n;memset(p,0,sizeof *p);p->pid=pid;snprintf(path,sizeof path,"/proc/%d/stat",pid);f=fopen(path,"r");if(!f)return -1;if(!fgets(line,sizeof line,f)){fclose(f);return -1;}fclose(f);close=strrchr(line,')');if(!close||close[1]!=' ')return -1;if(sscanf(line,"%d (%255[^)])",&n,p->name)!=2)return -1;char state;int parsed=sscanf(close+2,"%c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %llu %llu %lld %lld %lld %lld %lld %lld %llu",&state,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8],&v[9],&utime,&stime,&v[10],&v[11],&v[12],&v[13],&v[14],&v[15],&start);if(parsed<20)return -1;p->state=state;p->utime=utime;p->stime=stime;p->nice_value=(int)v[13];p->starttime=start;snprintf(path,sizeof path,"/proc/%d/status",pid);f=fopen(path,"r");if(f){unsigned uid;while(fgets(line,sizeof line,f))if(sscanf(line,"Uid:\t%u",&uid)==1){p->uid=(uid_t)uid;break;}fclose(f);}else return -1;p->valid=1;return 0;}
int process_identity_valid(const ManagedProcess*m,ProcessInfo*c){return process_read(m->info.pid,c)==0&&c->starttime==m->info.starttime&&c->uid==m->info.uid&&c->state!='Z';}
int process_is_owned(const ProcessInfo*p,uid_t uid){return p->uid==uid;}
int process_update_cpu(ManagedProcess*m,const ProcessInfo*p,double seconds){unsigned long long now=p->utime+p->stime,old=m->last_utime+m->last_stime;if(m->has_cpu_sample&&now>=old&&seconds>0)m->info.recent_cpu_percent=100.0*(double)(now-old)/(double)sysconf(_SC_CLK_TCK)/seconds;m->last_utime=p->utime;m->last_stime=p->stime;m->has_cpu_sample=1;m->info=*p;return 0;}
