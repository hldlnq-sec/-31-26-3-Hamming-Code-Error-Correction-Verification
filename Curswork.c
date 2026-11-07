#include <stdio.h>
#include "Hamming.h"
#include "Experiments.h"

#define N_BITS 31
#define K_BITS 26

int main(void)
{
    uint32_t information_word = 13u;
    uint32_t codeword = encode_hamming(information_word);

    printf("Information word: %u\n", (unsigned int)information_word);

    printf("Encoded codeword: %u\n", (unsigned int)codeword);

    for (unsigned int p = 1u; p <= CODE_BITS; p *= 2u)
    {
        int parity = calculate_parity_bit(codeword, p);
        int actual = get_bit(codeword, p - 1u);

        printf("Position %u: calculated = %d, actual = %d\n", p, parity, actual);
    }
    return 0;
}