#include <stdint.h>
#include <stddef.h>

void ntt_forward(uint64_t *data, size_t len);
void ntt_inverse(uint64_t *data, size_t len);
void ntt_pointwise(uint64_t *a, uint64_t *b, size_t len);
