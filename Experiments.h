
#include <stdint.h>
#include <stdlib.h>
#include "Hamming.h"

#define INFO_BITS 26u
#define PARITY_BITS 5u
#define CODE_BITS 31u

void init_random_generator(unsigned int seed);

int apply_bsc_channel(uint32_t codeword, double p, uint32_t *received_word);