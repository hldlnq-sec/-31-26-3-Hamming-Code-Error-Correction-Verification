
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

void init_random_generator(unsigned int seed);

int apply_bsc_channel(uint32_t codeword,unsigned int bit_count, double p, uint32_t *received_word);
int generate_error_pattern(unsigned int weight, uint32_t *error_pattern);
int apply_error_pattern(uint32_t codeword, uint32_t error_pattern, uint32_t *received_word);
int  count_bit_errors(uint32_t original_word, uint32_t received_word, unsigned int bit_count);

int run_bsc_experiment (double p, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], BSCExperimentResult *result);
int run_bsc_experiments(const double probabilities[], unsigned int probability_count, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], BSCExperimentResult results[]);
int run_fixed_weight_experiment(unsigned int weight, uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], FixedWeightExperimentResult *result);
int run_fixed_weight_experiments(uint64_t trials, const uint8_t H[PARITY_BITS][CODE_BITS], FixedWeightExperimentResult results[]);