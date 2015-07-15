#include <stdint.h>

#define MAX_TWIDDLE_TABLES 48
extern uint64_t *twiddles[MAX_TWIDDLE_TABLES];

void ensure_twiddle(size_t len);
