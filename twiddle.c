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

  for(size_t i=0;i<=intlog2(len);i++) {
    #ifdef VERBOSE
    printf("Generating twiddles %ju/%d...\n", i, intlog2(len));
    #endif

    if(twiddles[i] == NULL) {
      twiddles[i] = malloc(sizeof(uint64_t) << i);
      uint64_t twiddle = modexp(omega, 1ULL << (m - i), p);

      for(size_t j=0;j<(1 << i);j++) {
        if(j%2 == 0 && i > 2) {
          twiddles[i][j] = twiddles[i-1][j/2];
        } else {
          twiddles[i][j] = modexp(twiddle, j, p);
        }
      }
    }
  }
}
