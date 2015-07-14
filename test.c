#include <stdio.h>
#include <stdint.h>
#include "bitreverse.h"
#include "ntt.h"

char qux[] = {0, 1};
char foo[] = {0, 2, 1, 3};
char bar[] = {0, 4, 2, 6, 1, 5, 3, 7};
char baz[] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};

void assert_ordered(char* arr, int size) {
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



  if(bitreverse64(0b1111000011001100101010100000101010110100110010101010101111000110) !=
      0b0110001111010101010100110010110101010000010101010011001100001111) {
    fprintf(stderr, "bit reverse failed\n");
  }

  bit_reverse(qux, sizeof(qux));
  bit_reverse(foo, sizeof(foo));
  bit_reverse(bar, sizeof(bar));
  bit_reverse(baz, sizeof(baz));

  assert_ordered(qux, sizeof(qux));
  assert_ordered(foo, sizeof(foo));
  assert_ordered(bar, sizeof(bar));
  assert_ordered(baz, sizeof(baz));
}
