#include <stdint.h>
#include "mod.h"

// Calculcates a^n % mod
uint64_t modexp(uint64_t a, uint64_t exp, uint64_t modulus) {
  uint64_t result = 1;

  // Slow, but needed to prevent overflow.
  __uint128_t base = a%modulus;

  while (exp > 0) {
    if (exp & 1) {
      result = modmul(base, result, modulus);
    }

    base = modmul(base, base, modulus);
    exp >>= 1;
  }

  return result;
}

// Calculcates a*b % mod
uint64_t modmul(uint64_t a, uint64_t b, uint64_t modulus) {
  return (__uint128_t)a * (__uint128_t)b % (__uint128_t)modulus;
}

// x86 magic for lazy people
uint32_t intlog2(const uint64_t x) {
  return (31 - __builtin_clz (x));
}

