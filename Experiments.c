#include "Experiments.h"
#include <corecrt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "Hamming.h"

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

//функция эксперимента с определенным количеством ошибок
int run_fixed_weight_experiment(unsigned int weight, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], FixedWeightExperimentResult *result)
{
    if (trials == 0) return -1;
    if (H == NULL) return -1;
    if (result == NULL) return -1;
    if (weight > CODE_BITS) return -1;
    if (trials > UINT64_MAX / INFO_BITS) return -1;
    
    FixedWeightExperimentResult temp = {0};
    
    temp.weight = weight;
    temp.trials = trials;
    
    const uint32_t information_word = UINT32_C(0x01234567);
    const uint32_t codeword = encode_hamming(information_word);
    
    uint32_t error_pattern = 0u;
    uint32_t received_codeword = 0u;
    uint32_t decoded_information_word = 0u;
    
    for (uint64_t trial = 0; trial < trials; trial++)
    {
        if (generate_error_pattern(weight, &error_pattern) != 0) return -1;
        
        if (apply_error_pattern(codeword, error_pattern, &received_codeword) != 0) return -1;
        
        if (decode_hamming(received_codeword, H, &decoded_information_word) < 0) return -1;

        int coded_fixed_bit_error = count_bit_errors(information_word,decoded_information_word, INFO_BITS);
        if (coded_fixed_bit_error < 0) return -1;

        temp.bit_errors += (uint64_t)coded_fixed_bit_error;
        if (coded_fixed_bit_error > 0)
        {
            temp.block_errors++;
        }
    }

    temp.ber = (double) temp.bit_errors / ((double) trials * (double) INFO_BITS);
    temp.bler = (double) temp.block_errors / (double) trials;

    *result = temp;

    return 0;
}

//моделирование канала с определенным количеством ошибок
int run_fixed_weight_experiments(uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], FixedWeightExperimentResult results[])
{
    if (trials == 0u) return -1;
    if (H == NULL) return -1;
    if (results == NULL) return -1;

    for (unsigned int weight = 0u; weight <= CODE_BITS; weight++)
    {
        if (run_fixed_weight_experiment(weight, trials, H, &results[weight]) != 0) return -1;
    }

    return 0;
}


int run_exhaustive_weight_experiment(unsigned int weight, const uint8_t H[PARITY_BITS][CODE_BITS], ExhaustiveWeightResult *result)
{
    if (H == NULL) return -1;
    if (result == NULL) return -1;
    if (weight > MAX_EXHAUSTIVE_WEIGHT) return -1;

    ExhaustiveWeightResult temp = {0};
    temp.weight = weight;

    const uint32_t information_word = UINT32_C(0x01234567);
    const uint32_t codeword = encode_hamming(information_word);
    
    uint32_t received_codeword = 0u;
    uint32_t decoded_information_word = 0u;

    switch (weight)
    {
        case 0:
        {
            uint32_t error_pattern = UINT32_C(0);

            if (apply_error_pattern(codeword, error_pattern, &received_codeword) != 0) return -1;
            
            if (decode_hamming(received_codeword, H, &decoded_information_word) < 0) return -1;

            int bit_error = count_bit_errors(information_word,decoded_information_word, INFO_BITS);
            if (bit_error < 0) return -1;

            temp.bit_errors += (uint64_t)bit_error;

            if (bit_error > 0)
            {
                temp.block_errors++;
            }

            temp.patterns_tested++;

            break;
        }

        case 1:
        {
            for (unsigned int i = 0u; i < CODE_BITS; i++)
            {
                uint32_t error_pattern = (UINT32_C(1) << i);
                
                if (apply_error_pattern(codeword, error_pattern, &received_codeword) != 0) return -1;
                
                if (decode_hamming(received_codeword, H, &decoded_information_word) < 0) return -1;

                int bit_error = count_bit_errors(information_word,decoded_information_word, INFO_BITS);
                if (bit_error < 0) return -1;

                temp.bit_errors += (uint64_t)bit_error;

                if (bit_error > 0)
                {
                    temp.block_errors++;
                }

                temp.patterns_tested++;
            }
            break;
        }

        case 2:
        {
            for (unsigned int i = 0u; i < CODE_BITS; i++)
            {
                for (unsigned int j = i + 1u; j < CODE_BITS; j++)
                {
                    
                    uint32_t error_pattern = (UINT32_C(1) << i) | (UINT32_C(1) << j);

                    if (apply_error_pattern(codeword, error_pattern, &received_codeword) != 0) return -1;
                    
                    if (decode_hamming(received_codeword, H, &decoded_information_word) < 0) return -1;

                    int bit_error = count_bit_errors(information_word,decoded_information_word, INFO_BITS);
                    if (bit_error < 0) return -1;

                    temp.bit_errors += (uint64_t)bit_error;

                    if (bit_error > 0)
                    {
                        temp.block_errors++;
                    }
                    
                    temp.patterns_tested++;
                }
            }
            break;
        }

        case 3:
        {
            for (unsigned int i = 0u; i < CODE_BITS; i++)
            {
                for (unsigned int j = i + 1u; j < CODE_BITS; j++)
                {
                    for (unsigned int k = j + 1u; k < CODE_BITS; k++)
                    {
                        uint32_t error_pattern = (UINT32_C(1) << i) | (UINT32_C(1) << j) | (UINT32_C(1) << k);
                        
                        temp.patterns_tested++;

                        if (apply_error_pattern(codeword, error_pattern, &received_codeword) != 0) return -1;
                        
                        if (decode_hamming(received_codeword, H, &decoded_information_word) < 0) return -1;

                        int bit_error = count_bit_errors(information_word,decoded_information_word, INFO_BITS);
                        if (bit_error < 0) return -1;

                        temp.bit_errors += (uint64_t)bit_error;

                        if (bit_error > 0)
                        {
                            temp.block_errors++;
                        }
                    }
                }
            }
            break;
        }

        default: return -1;
    }

    temp.ber = (double) temp.bit_errors / ((double) temp.patterns_tested * (double) INFO_BITS);
    temp.bler = (double) temp.block_errors / (double) temp.patterns_tested;

    *result = temp;

    return 0;
}

//
int run_exhaustive_experiment(const uint8_t H[PARITY_BITS][CODE_BITS], ExhaustiveWeightResult results[])
{
    if (H == NULL) return -1;
    if (results == NULL) return -1;

    for (unsigned int weight = 0u; weight <= MAX_EXHAUSTIVE_WEIGHT; weight++)
    {
        if (run_exhaustive_weight_experiment(weight, H, &results[weight]) != 0) return -1;
    }
    
    return 0;
}

//
int save_bsc_results_csv(const char *filename, const BSCExperimentResult results[], unsigned int result_count)
{
    if (filename == NULL || filename[0] == '\0') return -1;
    if (results == NULL) return -1;
    if (result_count == 0) return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) return -1;

    if (fprintf(file,"p;trials;uncoded_bit_errors;coded_bit_errors;uncoded_block_errors;coded_block_errors;uncoded_ber;coded_ber;uncoded_bler;coded_bler\n") < 0)
    {
        fclose(file);
        return -1;
    }

    for (unsigned int i = 0; i < result_count; i++)
    {
        if (fprintf(file,"%.10g;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";" "%" PRIu64 ";%" PRIu64 ";" "%.12g;%.12g;%.12g;%.12g\n",
        results[i].p,
        results[i].trials,
        results[i].uncoded_bit_errors,
        results[i].coded_bit_errors,
        results[i].uncoded_block_errors,
        results[i].coded_block_errors,
        results[i].uncoded_ber,
        results[i].coded_ber,
        results[i].uncoded_bler,
        results[i].coded_bler) < 0)
        {
            fclose(file);
            return -1;
        }
    }

    if (fclose(file) != 0) return -1;

    return 0;
}

//
int save_fixed_weight_results_csv(const char *filename, const FixedWeightExperimentResult results[], unsigned int result_count)
{
    if (filename == NULL || filename[0] == '\0') return -1;
    if (results == NULL) return -1;
    if (result_count == 0) return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) return -1;

    if (fprintf(file, "weight;trials;bit_errors;block_errors;ber;bler\n") < 0)
    {
        fclose(file);
        return -1;
    }

    for (unsigned int i = 0; i < result_count; i++)
    {
        if (fprintf(file,"%u;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%.12g;%.12g\n",
        results[i].weight,
        results[i].trials,
        results[i].bit_errors,
        results[i].block_errors,
        results[i].ber,
        results[i].bler) < 0)
        {
            fclose(file);
            return -1;
        }
    }

    if (fclose(file) != 0) return -1;

    return 0;
}

//
int save_exhaustive_results_csv(const char *filename, const ExhaustiveWeightResult results[], unsigned int result_count)
{
    if (filename == NULL || filename[0] == '\0') return -1;
    if (results == NULL) return -1;
    if (result_count == 0) return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) return -1;

    if (fprintf(file, "weight;patterns_tested;bit_errors;block_errors;ber;bler\n") < 0)
    {
        fclose(file);
        return -1;
    }

    for (unsigned int i = 0; i < result_count; i++)
    {
        if (fprintf(file, "%u;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%.12g;%.12g\n",
    results[i].weight,
    results[i].patterns_tested,
    results[i].bit_errors,
    results[i].block_errors,
    results[i].ber,
    results[i].bler) < 0)
        {
            fclose(file);
            return -1;
        }
    }

    if (fclose(file) != 0) return -1;

    return 0;
}