#include <stdio.h>
#include <unistd.h>
int main(void){printf("cpu_hog PID %d\n",getpid());fflush(stdout);volatile unsigned long x=0;for(;;)x=x*1664525u+1013904223u;return (int)x;}
