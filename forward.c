#include <stdio.h>
#include <stdlib.h>
#include "swap.h"

// Forward transform a given file using NTT

int main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s filename\n  Transforms a file using NTT\n", argv[0]);
    exit(1);
  }

  swap_ntt_forward(argv[1], 1024*1024); // 1 MB I/O buffer
}
