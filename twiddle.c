#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mod.h"
#include "twiddle.h"

#define VERBOSE

uint64_t *twiddles[MAX_TWIDDLE_TABLES];

void ensure_twiddle(size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p

  #ifdef VERBOSE
  printf("Generating twiddle table of size %ju...\n", len);
  #endif

  size_t k = intlog2(len);

  // Check if already generated
  if(twiddles[k] != NULL) {
    return;
  }

  twiddles[k] = malloc(sizeof(uint64_t) << k);
  twiddles[k][0] = 1;

  if(k == 0) {
    return;
  }

  uint64_t twiddle = modexp(omega, 1ULL << (m - k), p);
  twiddles[k][1] = twiddle;

  if(k == 1) {
    goto finish;
  }

  if(twiddles[k+1] == NULL) {
    for(size_t i=2;i<(1 << k);i++) {
      twiddles[k][i] = modmul(twiddles[k][i-1], twiddle, p);
    }
  } else {
    for(size_t i=2;i<(1 << k);i++) {
      twiddles[k][i] = twiddles[k+1][i*2];
    }
  }

  finish:
  ensure_twiddle(len/2);
}
