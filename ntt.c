#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "mod.h"
#include "twiddle.h"

void ntt_forward(uint64_t *data, size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;

  if(len == 1) {
    return;
  }

  uint64_t *twiddle = twiddles[intlog2(len)];

  for(size_t i=0;i<len/2;i++) {
    uint64_t a = data[i];
    uint64_t b = data[i+len/2];

    data[i]       = (a+b)%p;

    // FIXME: Prevent overflow, but bad for branch predition
    if(a < b) {
      a+=p;
    }

    data[i+len/2] = (a-b)%p;
    data[i+len/2] = modmul(data[i+len/2], twiddle[i], p);
  }

  ntt_forward(data,       len/2);
  ntt_forward(data+len/2, len/2);
}

void ntt_pointwise(uint64_t *a, uint64_t *b, size_t len) {
  const uint64_t p = 4179340454199820289;

  for(size_t i=0;i<len;i++) {
    a[i] = modmul(a[i], b[i], p);
  }
}

void ntt_inverse(uint64_t *data, size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t scale = 2089670227099910145;

  // Fast shortcut
  if(len == 1) {
    return;
  }

  ntt_inverse(data,       len/2);
  ntt_inverse(data+len/2, len/2);

  uint64_t *twiddle = twiddles[intlog2(len)];

  for(size_t i=0;i<len/2;i++) {
    uint64_t a = data[i];
    uint64_t b = modmul(data[i+len/2], twiddle[(len - i)%len], p);

    data[i]       = modmul(a+b,   scale, p);
    data[i+len/2] = modmul(a+p-b, scale, p);
  }
}
