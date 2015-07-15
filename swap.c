#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "ntt.h"
#include "mod.h"

void swap_ntt_forward(char* file, size_t bufsize) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p

  int fd = open(file, O_RDWR);

  if(fd == -1) {
    fprintf(stderr, "Cannot open file\n");
    exit(1);
  }

  size_t bufelems = bufsize/sizeof(uint64_t);
  uint64_t *buffer = malloc(bufsize);

  printf("Using %ju bytes I/O buffer\n", bufelems*sizeof(uint64_t));

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

  if(bufelems < rows) {
    fprintf(stderr, "Transform too large, buffer too small (FIXME: Recursive Swap NTT!)\n");
    exit(1);
  }
  size_t cols_per_read = bufelems/rows;
  printf("Can read %ju cols per read\n", cols_per_read);

  size_t iterations_over_file = cols/cols_per_read;

  for(size_t o=0;o<iterations_over_file;o++) {
    for(size_t q=0;q<rows;q++) {
      size_t source_offset   = q*cols+o*cols_per_read;
      size_t readsize = cols_per_read;
      size_t target_offset = q*cols_per_read;

      printf("Reading from %ju: %ju uint64_t's\n", source_offset, readsize);
      lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
      if(read(fd, buffer+target_offset, readsize*sizeof(uint64_t)) != readsize*sizeof(uint64_t)) {
        fprintf(stderr, "Read failed\n");
        exit(1);
      }
    }

    // Now transform all the columns
    uint64_t *col = malloc(rows*sizeof(uint64_t));
    for(size_t i=0;i<cols_per_read;i++) {
      printf("Column forward transform %ju/%ju in iteration %ju/%ju...\n", i, cols_per_read, o, iterations_over_file);
      for(size_t j=0;j<rows;j++) {
        col[j] = buffer[i+cols_per_read*j];
      }
      ntt_forward(col, rows);
      for(size_t j=0;j<rows;j++) {
        buffer[i+cols_per_read*j] = modmul(col[j], modexp(twiddle, (i+o*cols_per_read)*j, p), p);
      }
    }
    free(col);

    for(size_t q=0;q<rows;q++) {
      size_t source_offset   = q*cols+o*cols_per_read;
      size_t readsize = cols_per_read;
      size_t target_offset = q*cols_per_read;

      // And write the result
      printf("Writing to %ju: %ju uint64_t's\n", source_offset, readsize);
      lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
      if(write(fd, buffer+target_offset, readsize*sizeof(uint64_t)) != readsize*sizeof(uint64_t)) {
        fprintf(stderr, "Write failed\n");
        exit(1);
      }
    }
  }

  // Now do the same on the rows
  size_t rows_per_read = bufelems/cols;
  iterations_over_file = rows/rows_per_read;

  for(size_t o=0;o<iterations_over_file;o++) {
    size_t source_offset   = o*cols*rows_per_read;
    size_t readsize = bufelems;

    printf("Reading from %ju: %ju uint64_t's\n", source_offset, readsize);
    lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
    if(read(fd, buffer, readsize*sizeof(uint64_t)) != readsize*sizeof(uint64_t)) {
      fprintf(stderr, "Read failed\n");
      exit(1);
    }

    // Transform
    for(size_t i=0;i<rows_per_read;i++) {
      printf("Row forward transform %ju/%ju in iteration %ju/%ju...\n", i, rows_per_read, o, iterations_over_file);
      ntt_forward(buffer+i*cols, cols);
    }

    // Write
    printf("Writing to %ju: %ju uint64_t's\n", source_offset, readsize);
    lseek(fd, source_offset*sizeof(uint64_t), SEEK_SET);
    if(write(fd, buffer, readsize*sizeof(uint64_t)) != readsize*sizeof(uint64_t)) {
      fprintf(stderr, "Write failed\n");
      exit(1);
    }
  }

  close(fd);
}
