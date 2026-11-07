#include <stdint.h>
#include <stdlib.h>
#include "Hamming.h"

#define INFO_BITS 26u
#define PARITY_BITS 5u
#define CODE_BITS 31u

//инициализация генератора псевдослучайных чисел
void init_random_generator(unsigned int seed)
{
    srand(seed);
}

//создание ошибок при прохождении канала
int apply_bsc_channel(uint32_t codeword, double p, uint32_t *received_word)
{
    if (((codeword >> CODE_BITS) != 0u) || (p <= 0) || (p >= 1) || (received_word == NULL)) return -1;

    uint32_t result = codeword;

    for (unsigned int position = 0; position < CODE_BITS; position++)
    {
        double random_value = (double)rand() / ((double)RAND_MAX + 1.0);

        if (random_value < p)
        {
            result = toggle_bit(result, position);
        }
    }

    *received_word = result;

    return 0;
}

//создание случайной маски для точного количества ошибок
int generate_error_pattern(unsigned int weight, uint32_t *error_pattern)
{
    if (error_pattern == NULL || weight > CODE_BITS) return -1;

    uint32_t pattern = 0u;
    unsigned int generated_errors = 0u;
    const uint64_t random_range = (uint64_t)RAND_MAX + 1u;
    const uint64_t usable_range = random_range - (random_range % CODE_BITS);

    while (generated_errors < weight)
    {
        uint64_t random_number;

        do
        {
            random_number = (uint64_t)rand();
        } while (random_number >= usable_range);

        unsigned int position = (unsigned int)(random_number % CODE_BITS);

        if (get_bit(pattern, position) == 0)
        {
            pattern = set_bit(pattern, position);

            generated_errors++;
        }
    }

    *error_pattern = pattern;

    return 0;
}

//применение маски ошибок к слову
int apply_error_pattern(uint32_t codeword, uint32_t error_pattern, uint32_t *received_word)
{
    if ((codeword >> CODE_BITS) != 0u || (error_pattern >> CODE_BITS) != 0u || received_word == NULL)
    {
        return -1;
    }

    uint32_t result = codeword ^ error_pattern;

    *received_word = result;

    return 0;
}

//подсчет количества ошибок
int  count_bit_errors()