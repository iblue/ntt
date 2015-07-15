#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "ntt.h"
#include "bitreverse.h"
#include "mod.h"

void baileys_forward(uint64_t *data, size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p

  size_t k  = intlog2(len);
  size_t k1 = k/2;
  size_t k2 = k - k1;
  size_t rows = 1 << k1;
  size_t cols = 1 << k2;

  uint64_t twiddle = modexp(omega, 1ULL << (m - k), p);

  // Do NTT on the cols (FIXME: PoC, but totally inefficient shit)
  for(size_t i=0;i<cols;i++) {
    uint64_t *col = malloc(rows*sizeof(uint64_t));
    for(size_t j=0;j<rows;j++) {
      col[j] = data[i+cols*j];
    }
    ntt_forward(col, rows);
    //bit_reverse(col, rows);
    for(size_t j=0;j<rows;j++) {
      data[i+cols*j] = modmul(col[j], modexp(twiddle, i*j, p), p);
    }
  }

  // Do NTT on the rows
  for(size_t i=0;i<rows;i++) {
    ntt_forward(data+i*cols, cols);
    //bit_reverse(data+i*cols, cols); // FIXME: Not needed for convolution
  }
}

void baileys_inverse(uint64_t *data, size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p

  size_t k  = intlog2(len);
  size_t k1 = k/2;
  size_t k2 = k - k1;
  size_t rows = 1 << k1;
  size_t cols = 1 << k2;

  uint64_t twiddle = modexp(omega, 1ULL << (m - k), p);

  // Do inverse NTT on the rows
  for(size_t i=0;i<rows;i++) {
    ntt_inverse(data+i*cols, cols);
    //bit_reverse(data+i*cols, cols); // FIXME: Not needed for convolution
  }

  // Do NTT on the cols (FIXME: PoC, but totally inefficient shit)
  for(size_t i=0;i<cols;i++) {
    uint64_t *col = malloc(rows*sizeof(uint64_t));
    for(size_t j=0;j<rows;j++) {
      col[j] = modmul(data[i+cols*j], modexp(twiddle, len-i*j, p) ,p);
    }
    ntt_inverse(col, rows);
    //bit_reverse(col, rows);
    for(size_t j=0;j<rows;j++) {
      data[i+cols*j] = col[j];
    }
  }
}
