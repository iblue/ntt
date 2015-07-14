#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "ntt.h"
#include "bitreverse.h"

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
    bit_reverse(col, rows);
    for(size_t j=0;j<rows;j++) {
      data[i+cols*j] = modmul(col[j], modexp(twiddle, i*j, p), p);
    }
  }

  // Do NTT on the rows
  for(size_t i=0;i<rows;i++) {
    ntt_forward(data+i*cols, cols);
    bit_reverse(data+i*cols, cols); // FIXME: Not needed for convolution
  }
}
