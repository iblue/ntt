#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "swap.h"
#include "twiddle.h"

#define VERBOSE

double wall_clock() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec + 1.0e-9*t.tv_nsec;
}

// Forward transform a given file using NTT
int main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s filename\n  Transforms a file using NTT\n", argv[0]);
    exit(1);
  }

  double time0 = wall_clock();

  ensure_twiddle(1024*1024*16);

  double time1 = wall_clock();

  swap_ntt_forward(argv[1], 1024*1024); // 1 MB I/O buffer

  double time2 = wall_clock();

  printf("Twiddle Generation: %f sec, NTT: %f sec\n", time1 - time0, time2 - time1);
}
