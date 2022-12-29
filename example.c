#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

void x(void *k) {
  uint64_t l;

  for (l = 0; l < 10000000; l++);

  yld();

  for (l = 0; l < 10000000; l++);
}

void y(void *k) {
  for (int i = 0; i < 1000; i ++) {
    task(x, NULL);
    yld();
  }
}

void z(void *k) {
  printf("I'm Zed\n");
}

int main() {
  if (init())
    exit(EXIT_FAILURE);

  task(x, NULL);
  task(y, NULL);
  task(z, NULL);
  run();
  cleanup();
  return 0;
}
