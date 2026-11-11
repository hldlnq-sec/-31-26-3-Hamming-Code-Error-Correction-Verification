#include <corecrt.h>
#include <stdint.h>
#include <stdlib.h>
#include "Hamming.h"

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

typedef struct
{
    unsigned int weight;
    uint64_t trials;
    
    uint64_t bit_errors;
    uint64_t block_errors;
    
    double ber;
    double bler;
    
} FixedWeightExperimentResult;

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
    if (H == NULL) return -1;
    
    BSCExperimentResult temp = {0};
    
    temp.p = p;
    temp.trials = trials;
    
    const uint32_t information_word = UINT32_C(0x01234567);
    const uint32_t codeword = encode_hamming(information_word);
    
    for (uint64_t trial = 0u; trial < trials; trial++)
    {
        uint32_t received_information_word = 0u;
        
        if (apply_bsc_channel(information_word,INFO_BITS, p, &received_information_word) != 0)
        {
            return -1;
        }
        
        int uncoded_errors = count_bit_errors(information_word, received_information_word, INFO_BITS);
        if (uncoded_errors < 0)
        {
            return -1;
        }
        
        temp.uncoded_bit_errors += (uint64_t)uncoded_errors;
        if (uncoded_errors > 0)
        {
            temp.uncoded_block_errors++;
        }
        uint32_t received_codeword = 0u;
        
        
        if (apply_bsc_channel(codeword, CODE_BITS, p, &received_codeword) != 0)
        {
            return -1;
        }
        
        uint32_t decoded_information_word = 0u;
        
        int decode_status = decode_hamming(received_codeword, H, &decoded_information_word);
        
        if (decode_status < 0) return -1;
        
        int coded_errors = count_bit_errors(information_word, decoded_information_word, INFO_BITS);
        
        if (coded_errors < 0) return -1;
        
        temp.coded_bit_errors += (uint64_t)coded_errors;
        if (coded_errors > 0)
        {
            temp.coded_block_errors++;
        }
    }
    
    temp.uncoded_ber = (double)temp.uncoded_bit_errors / ((double)trials * INFO_BITS);
    temp.coded_ber = (double)temp.coded_bit_errors / ((double)trials * INFO_BITS);
    temp.uncoded_bler = (double)temp.uncoded_block_errors / ((double)trials);
    temp.coded_bler = (double)temp.coded_block_errors / ((double)trials);
    
    *result = temp;
    
    return 0;
}

//моделирование канала с разными вероятностями
int run_bsc_experiments(const double probabilities[], unsigned int probability_count, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], BSCExperimentResult results[])
{
    if (probabilities == NULL) return -1;
    if (probability_count == 0u) return -1;
    if (trials == 0u) return -1;
    if (H == NULL) return -1;
    if (results == NULL) return -1;
    
    for (unsigned int i = 0; i < probability_count; i++)
    {
        if (run_bsc_experiment(probabilities[i], trials, H, &results[i]) != 0) return -1;
    }
    
    return 0;
}