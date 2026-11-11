#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Hamming.h"
#include "Experiments.h"

int main(void)
{
    /* Матрицы кода Хэмминга [31, 26, 3] */
    uint8_t G[INFO_BITS][CODE_BITS];
    uint8_t H[PARITY_BITS][CODE_BITS];

    /* Построение матриц */
    build_generator_matrix(G);
    build_check_matrix(H);

    /* Проверка матриц */
    if (!verify_matrices(G, H))
    {
        printf("Matrix verification failed!\n");
        return EXIT_FAILURE;
    }

    printf("Matrices verified successfully!\n");

    /* Инициализация генератора случайных чисел */
    init_random_generator(12345u);

    /* Результаты эксперимента */
    BSCExperimentResult result;

    const uint64_t trials = 1000u;
    const double probabilities[] = {0.0, 1.0};

    /* Проверка двух граничных вероятностей */
    for (unsigned int i = 0u; i < 2u; i++)
    {
        double p = probabilities[i];

        printf("\n================================\n");
        printf("Experiment: p = %.2f\n", p);
        printf("================================\n");

        if (run_bsc_experiment(p, trials, H, &result) != 0)
        {
            printf("Experiment failed!\n");
            return EXIT_FAILURE;
        }

        printf("\n--- Uncoded transmission ---\n");

        printf("Bit errors:   %llu\n", (unsigned long long)result.uncoded_bit_errors);

        printf("Block errors: %llu\n", (unsigned long long)result.uncoded_block_errors);

        printf("BER: %.6f\n", result.uncoded_ber);
        printf("BLER: %.6f\n", result.uncoded_bler);

        printf("\n--- Coded transmission ---\n");

        printf("Bit errors:   %llu\n", (unsigned long long)result.coded_bit_errors);

        printf("Block errors: %llu\n", (unsigned long long)result.coded_block_errors);

        printf("BER: %.6f\n", result.coded_ber);
        printf("BLER: %.6f\n", result.coded_bler);
    }

    printf("\nAll tests completed successfully!\n");

    return EXIT_SUCCESS;
}