#include "autonicer.h"
#include "monitor.h"
#include "process.h"
#include "memory.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
int main(void){SystemSample s;ProcessInfo p;MemorySample m;assert(monitor_read(&s)==0);assert(s.total>s.idle);assert(process_read(getpid(),&p)==0);assert(p.pid==getpid());assert(p.starttime>0);assert(p.state!='Z');assert(memory_read(&m)==0);assert(m.total_kb>0);assert(m.available_kb<=m.total_kb);puts("unit tests passed");return 0;}
