#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Hamming.h"
#include "Experiments.h"

int main(void)
{
    BSCExperimentResult bsc_results[9];
    FixedWeightExperimentResult fixed_results[CODE_BITS + 1u];
    ExhaustiveWeightResult exhaustive_results[MAX_EXHAUSTIVE_WEIGHT + 1u];

    const double probabilities[] = {0.001, 0.0025, 0.005, 0.01, 0.02, 0.05, 0.10, 0.15, 0.20};

    const uint64_t bsc_trials = 200000u;
    const uint64_t fixed_weight_trials = 25000u;

    
}