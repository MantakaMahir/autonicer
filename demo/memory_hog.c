#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  size_t mb = argc > 1 ? (size_t)strtoull(argv[1], NULL, 10) : 128;
  size_t bytes = mb * 1024 * 1024;
  unsigned char *p = malloc(bytes);
  if (!p) {
    perror("malloc");
    return 1;
  }
  for (size_t i = 0; i < bytes; i += 4096)
    p[i] = (unsigned char)i;
  printf("memory_hog PID %d allocated %zu MB\n", getpid(), mb);
  fflush(stdout);
  for (;;) {
    for (size_t i = 0; i < bytes; i += 4096)
      p[i]++;
    sleep(1);
  }
  return 0;
}
