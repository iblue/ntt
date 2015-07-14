#include <stdint.h>
#include <stddef.h>

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

// x86 magic for lazy people
static inline uint32_t intlog2(const uint64_t x) {
  return (31 - __builtin_clz (x));
}

void ntt_forward(uint64_t *data, size_t len) {
  // Determined using nttgen
  const uint64_t p     = 4179340454199820289;
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p
  //const uint64_t scale = 2089670227099910145;

  if(len == 1) {
    return;
  }

  for(size_t i=0;i<len/2;i++) {
    uint64_t twiddle = modexp(omega, 1ULL << (m - intlog2(len)), p);
    uint64_t a = data[i];
    uint64_t b = data[i+len/2];

    data[i]       = (a+b)%p;

    // FIXME: Prevent overflow, but bad for branch predition
    if(a < b) {
      a+=p;
    }

    data[i+len/2] = (a-b)%p;
    data[i+len/2] = modmul(data[i+len/2], modexp(twiddle, i, p), p);
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
  const uint64_t omega = 68630377364883;
  const uint64_t m     = 57; // omega^(2^57) = 1 mod p
  const uint64_t scale = 2089670227099910145;

  if(len == 1) {
    return;
  }

  ntt_inverse(data,       len/2);
  ntt_inverse(data+len/2, len/2);

  for(size_t i=0;i<len/2;i++) {
    uint64_t twiddle = modexp(omega, 1ULL << (m - intlog2(len)), p);;
    uint64_t a = data[i];
    uint64_t b = modmul(data[i+len/2], modexp(twiddle, len - i, p), p);

    data[i]       = (a+b)%p;

    // FIXME: Prevent overflow, but bad for branch predition
    if(a < b) {
      a+=p;
    }

    data[i+len/2] = (a-b)%p;

    data[i]       = modmul(data[i],       scale, p);
    data[i+len/2] = modmul(data[i+len/2], scale, p);
  }
}
