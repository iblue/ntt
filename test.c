#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "bitreverse.h"
#include "ntt.h"
#include "baileys.h"
#include "swap.h"
#include "twiddle.h"

uint64_t qux[] = {0, 1};
uint64_t foo[] = {0, 2, 1, 3};
uint64_t bar[] = {0, 4, 2, 6, 1, 5, 3, 7};
uint64_t baz[] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};

void assert_ordered(uint64_t* arr, int size) {
  for(int i=0;i<size;i++) {
    if(i != arr[i]) {
      goto fail;
    }
  }
  return;
  fail:
  fprintf(stderr, "error: unexpected ordering\n");
}

int main(void) {
  ensure_twiddle(1024);

  uint64_t data[] = {23, 42};
  ntt_forward(data, 2);
  if(data[0] != 65 || data[1] != 4179340454199820270) {
    fprintf(stderr, "error: NTT[2] failed\n");
  }

  uint64_t data2[] = {23, 42, 0, 0};
  ntt_forward(data2, 4);
  if(data2[0] != 65 || data2[1] != 4179340454199820270 || data2[2] != 3204538169783827610 || data2[3] != 974802284415992725) {
    fprintf(stderr, "error: NTT[4] failed\n");
  }

  ntt_inverse(data2, 4);
  if(data2[0] != 23 || data2[1] != 42 || data2[2] != 0 || data2[3] != 0) {
    fprintf(stderr, "error: INTT[4] failed\n");
  }

  uint64_t data3[1024];
  uint64_t copy[1024];
  for(int i=0;i<1024;i++) {
    copy[i] = data3[i] = (rand()+rand()*(1ULL << 32))%(1ULL << 61);
  }
  ntt_forward(data3, 1024);
  ntt_inverse(data3, 1024);
  for(int i=0;i<1024;i++) {
    if(copy[i] != data3[i]) {
      fprintf(stderr, "error: NTT or INTT 1024 failed\n");
      break;
    }
  }

  // Small Baileys test
  uint64_t btest1[] = {1, 2, 8, 17, 4, 0, 0, 0};
  uint64_t btest2[] = {1, 2, 8, 17, 4, 0, 0, 0};

  ntt_forward(btest1, 8);
  //bit_reverse(btest1, 8);
  baileys_forward(btest2, 8);
  for(int i=0;i<sizeof(btest1)/sizeof(uint64_t);i++) {
    if(btest1[i] != btest2[i]) {
      fprintf(stderr, "error: Baileys failed\n");
      break;
    }
  }

  ntt_inverse(btest1, 8);
  baileys_inverse(btest2, 8);
  for(int i=0;i<sizeof(btest1)/sizeof(uint64_t);i++) {
    if(btest1[i] != btest2[i]) {
      fprintf(stderr, "error: Inverse Baileys failed\n");
      break;
    }
  }

  // Large Baileys test
  for(int i=0;i<1024;i++) {
    copy[i] = data3[i] = (rand()+rand()*(1ULL << 32))%(1ULL << 61);
  }
  baileys_forward(data3, 1024);
  baileys_inverse(data3, 1024);
  for(int i=0;i<1024;i++) {
    if(copy[i] != data3[i]) {
      fprintf(stderr, "error: NTT or INTT 1024 failed\n");
      break;
    }
  }

  // Multiplication tests
  {
    #define LEN 1024
    uint64_t a[LEN] = {23, 42};
    uint64_t b[LEN] = {10, 11};
    ntt_forward(a, LEN);
    ntt_forward(b, LEN);
    ntt_pointwise(a, b, LEN);
    ntt_inverse(a, LEN);
    for(size_t i=3;i<LEN;i++) {
      if(a[i] != 0) {
        fprintf(stderr, "NTT Multiplication failed\n");
        break;
      }
    }
    if(a[0] != 230 || a[1] != 673 || a[2] != 462) {
      fprintf(stderr, "NTT Multiplication failed\n");
    }
  }
  {
    #define LEN 1024
    uint64_t a[LEN] = {23, 42};
    uint64_t b[LEN] = {10, 11};
    baileys_forward(a, LEN);
    baileys_forward(b, LEN);
    ntt_pointwise(a, b, LEN);
    baileys_inverse(a, LEN);
    for(size_t i=3;i<LEN;i++) {
      if(a[i] != 0) {
        fprintf(stderr, "Baileys Multiplication failed\n");
        break;
      }
    }
    if(a[0] != 230 || a[1] != 673 || a[2] != 462) {
      fprintf(stderr, "Baileys Multiplication failed\n");
    }
  }

  // Test swap mode
  {
    uint64_t swapdata[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    uint64_t result[32];
    FILE* fh = fopen("example.ntt", "wb");
    fwrite(swapdata, 32, sizeof(uint64_t), fh);
    fclose(fh);
    swap_ntt_forward("example.ntt", 64);
    baileys_forward(swapdata, 32);
    fh = fopen("example.ntt", "rb");
    // Silence compiler warnings. We will check by reading result anyway.
    if(fread(result, 32, sizeof(uint64_t), fh));
    fclose(fh);

    for(int i=0;i<32;i++) {
      if(swapdata[i] != result[i]) {
        fprintf(stderr, "error: Swap mode 128 failed\n");
        break;
      }
    }
  }

  // Test twiddle generation
  {
    ensure_twiddle(128);

    if(twiddles[0][0] != 1) {
      fprintf(stderr, "ensure_twiddle[0] failed\n");
    }
    if(twiddles[1][0] != 1 || twiddles[1][1] != 4179340454199820288) {
      fprintf(stderr, "ensure_twiddle[1] failed\n");
    }
    if(twiddles[2][0] != 1 || twiddles[2][1] != 3360066027580426122 ||
       twiddles[2][2] != 4179340454199820288 || twiddles[2][3] != 819274426619394167) {
      fprintf(stderr, "ensure_twiddle[2] failed\n");
    }
  }


  if(bitreverse64(0b1111000011001100101010100000101010110100110010101010101111000110) !=
      0b0110001111010101010100110010110101010000010101010011001100001111) {
    fprintf(stderr, "bit reverse failed\n");
  }

  bit_reverse(qux, sizeof(qux)/sizeof(uint64_t));
  bit_reverse(foo, sizeof(foo)/sizeof(uint64_t));
  bit_reverse(bar, sizeof(bar)/sizeof(uint64_t));
  bit_reverse(baz, sizeof(baz)/sizeof(uint64_t));

  assert_ordered(qux, sizeof(qux)/sizeof(uint64_t));
  assert_ordered(foo, sizeof(foo)/sizeof(uint64_t));
  assert_ordered(bar, sizeof(bar)/sizeof(uint64_t));
  assert_ordered(baz, sizeof(baz)/sizeof(uint64_t));
}
