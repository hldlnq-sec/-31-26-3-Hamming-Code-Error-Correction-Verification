#include <stdint.h>
#include <stdlib.h>
#include "Hamming.h"

#define INFO_BITS 26u
#define PARITY_BITS 5u
#define CODE_BITS 31u

typedef struct
{
    double p;
    uint64_t trials;

    uint64_t uncoded_bit_errors;
    uint64_t coded_bit_errors;

    uint64_t uncoded_block_errors;
    uint64_t coded_block_errors;

    double uncoded_ber;
    double coded_ber;

    double uncoded_bler;
    double coded_bler;

} BSCExperimentResult;

//инициализация генератора псевдослучайных чисел
void init_random_generator(unsigned int seed)
{
    srand(seed);
}

//создание ошибок при прохождении канала
int apply_bsc_channel(uint32_t codeword,unsigned int bit_count, double p, uint32_t *received_word)
{
    if (received_word == NULL) return -1;
    if (bit_count == 0u || bit_count > CODE_BITS) return -1;
    if ((codeword >> bit_count) != 0u) return -1;
    if (!(p >= 0.0 && p <= 1.0)) return -1;
    
    uint32_t result = codeword;

    for (unsigned int position = 0; position < bit_count; position++)
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
int  count_bit_errors(uint32_t original_word, uint32_t received_word, unsigned int bit_count)
{
    if (bit_count == 0u || bit_count > CODE_BITS) return -1;

    if ((original_word >> bit_count) != 0u || (received_word >> bit_count) != 0u) return -1;

    uint32_t difference = original_word ^ received_word;

    unsigned int errors = 0u;

    for (unsigned int bit_index = 0u; bit_index < bit_count; bit_index++)
    {
        if (get_bit(difference, bit_index)) errors++;
    }

    return errors;
}

//функция экспериментов
int run_bsc_experiment (double p, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], BSCExperimentResult *result)
{
    if (result == NULL) return -1;
    if (trials == 0) return -1;
    if (!(p >= 0 && p <= 1)) return -1;
    if (trials > UINT64_MAX / INFO_BITS) return -1;

    BSCExperimentResult temp = {0};

    temp.p = p;
    temp.trials = trials;

    const uint32_t information_word = UINT32_C(0x01234567);
    const uint32_t codeword = encode_hamming(information_word);

    for (unsigned int trial = 0u; trial < trials; trial++)
    {
        uint32_t received_information_word = 0u;

        apply_bsc_channel(information_word, INFO_BITS, p, &received_information_word);  

        
    }
}
