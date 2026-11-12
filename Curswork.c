#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Hamming.h"
#include "Experiments.h"

#define BSC_FILE "bsc_results.csv"
#define FIXED_WEIGHT_FILE "fixed_weight_results.csv"
#define EXHAUSTIVE_FILE "exhaustive_results.csv"

int main(void)
{
    enum { PROBABILITY_COUNT = 9 };
    const double probabilities[PROBABILITY_COUNT] = {0.001, 0.0025, 0.005, 0.01, 0.02, 0.05, 0.10, 0.15, 0.20};
    BSCExperimentResult bsc_results[PROBABILITY_COUNT];

    FixedWeightExperimentResult fixed_results[CODE_BITS + 1u];
    ExhaustiveWeightResult exhaustive_results[MAX_EXHAUSTIVE_WEIGHT + 1u];

    const uint64_t bsc_trials = 200000u;
    const uint64_t fixed_weight_trials = 25000u;

    uint8_t G[INFO_BITS][CODE_BITS] = {0};
    build_generator_matrix(G);

    uint8_t H[PARITY_BITS][CODE_BITS] = {0};
    build_check_matrix(H);

    if (!verify_matrices(G, H))
    {
        printf("Matrix verification failed!\n");
        return EXIT_FAILURE;
    }

    init_random_generator(1234);

    printf("\nStarting BSC experiments...\n");
    if (run_bsc_experiments(probabilities, PROBABILITY_COUNT, bsc_trials, H, bsc_results) != 0)
    {
        fprintf(stderr, "BSC experiment failed!\n");
        return EXIT_FAILURE;
    }
    if (save_bsc_results_csv(BSC_FILE, bsc_results, PROBABILITY_COUNT) != 0)
    {
        fprintf(stderr, "Failed to save BSC results!\n");
        return EXIT_FAILURE;
    }
    printf("BSC experiments completed.\n");

    printf("\nStarting Fixed weight experiments...\n");
    if (run_fixed_weight_experiments(fixed_weight_trials, H, fixed_results) != 0)
    {
        fprintf(stderr, "Fixed weight experiment failed!\n");
        return EXIT_FAILURE;
    }
    if (save_fixed_weight_results_csv(FIXED_WEIGHT_FILE, fixed_results, CODE_BITS + 1u) != 0)
    {
        fprintf(stderr, "Failed to save Fixed weight results!\n");
        return EXIT_FAILURE;
    }
    printf("Fixed weight experiments completed.\n");

    printf("\nStarting Exhaustive experiments...\n");
    if (run_exhaustive_experiment(H, exhaustive_results) != 0)
    {
        fprintf(stderr, "Exhaustive experiment failed!\n");
        return EXIT_FAILURE;
    }
    if (save_exhaustive_results_csv(EXHAUSTIVE_FILE, exhaustive_results, MAX_EXHAUSTIVE_WEIGHT + 1u) != 0)
    {
        fprintf(stderr, "Failed to save Exhaustive results!\n");
        return EXIT_FAILURE;
    }
    printf("Exhaustive experiments completed.\n");

    return EXIT_SUCCESS;
}