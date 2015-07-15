#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "ntt.h"

// x86 magic for lazy people
static inline uint32_t intlog2(const uint64_t x) {
  return (31 - __builtin_clz (x));
}

// Calculcates a^n % mod
static uint64_t modexp(uint64_t a, uint64_t exp, uint64_t modulus) {
  uint64_t result = 1;

  // Slow, but needed to prevent overflow.
  __uint128_t base = a%modulus;

  while (exp > 0) {
    if (exp & 1) {
      result = (result * base) % modulus;
    }

    base = (base * base) % modulus;
    exp >>= 1;
  }

  return result;
}

// Calculcates a*b % mod
static uint64_t modmul(uint64_t a, uint64_t b, uint64_t modulus) {
  return (__uint128_t)a * (__uint128_t)b % (__uint128_t)modulus;
}

void swap_ntt_forward(char* file) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p

  int fd = open(file, O_RDWR);

  if(fd == -1) {
    fprintf(stderr, "Cannot open file\n");
    exit(1);
  }

  #define BUFSIZE 8
  uint64_t buffer[BUFSIZE];

  printf("Using %ju bytes I/O buffer\n", BUFSIZE*sizeof(uint64_t));

  // Get file length
  off_t len = lseek(fd, 0, SEEK_END)/sizeof(uint64_t);
  lseek(fd, 0, SEEK_SET);

  printf("Reading %ju bytes file\n", len*sizeof(uint64_t));

  // Calculate number of rows and cols
  size_t k    = intlog2(len);
  size_t k1   = k/2;
  size_t k2   = k - k1;
  size_t rows = 1 << k1;
  size_t cols = 1 << k2;

  uint64_t twiddle = modexp(omega, 1ULL << (m - k), p);

  printf("Using file %s (len %ju) as %jux%ju matrix\n", file, len, rows, cols);

  if(BUFSIZE < rows) {
    fprintf(stderr, "Transform too large, buffer too small (FIXME: Recursive Swap NTT!)\n");
    exit(1);
  }
  size_t cols_per_read = BUFSIZE/rows;
  printf("Can read %ju cols per read\n", cols_per_read);

  size_t iterations_over_file = cols/cols_per_read;

  for(size_t o=0;o<iterations_over_file;o++) {
    for(size_t q=0;q<rows;q++) {
      size_t source_offset   = q*cols+o*cols_per_read;
      size_t readsize = cols_per_read;
      size_t target_offset = o*cols_per_read;

      printf("Reading from %ju: %ju uint64_t's\n", source_offset, readsize);
      lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
      read(fd, buffer+target_offset, readsize*sizeof(uint64_t));

      // Now transform all the columns
      uint64_t *col = malloc(rows*sizeof(uint64_t));
      for(size_t i=0;i<cols_per_read;i++) {
        for(size_t j=0;j<rows;j++) {
          col[j] = buffer[i+cols_per_read*j];
        }
        ntt_forward(col, rows);
        for(size_t j=0;j<rows;j++) {
          buffer[i+cols_per_read*j] = modmul(col[j], modexp(twiddle, i*j, p), p);
        }
      }
      free(col);

      // And write the result
      printf("Writing to %ju: %ju uint64_t's\n", source_offset, readsize);
      lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
      write(fd, buffer+target_offset, readsize*sizeof(uint64_t));
    }
  }

  // Now do the same on the rows
  size_t rows_per_read = BUFSIZE/cols;
  iterations_over_file = rows/rows_per_read;

  for(size_t o=0;o<iterations_over_file;o++) {
    size_t source_offset   = o*cols*rows_per_read;
    size_t readsize = BUFSIZE;

    printf("Reading from %ju: %ju uint64_t's\n", source_offset, readsize);
    lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
    read(fd, buffer, readsize*sizeof(uint64_t));

    // Transform
    for(size_t i=0;i<rows_per_read;i++) {
      ntt_forward(buffer+i*cols, cols);
    }

    // Write
    printf("Writing to %ju: %ju uint64_t's\n", source_offset, readsize);
    lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
    write(fd, buffer, readsize*sizeof(uint64_t));
  }

  close(fd);
}
