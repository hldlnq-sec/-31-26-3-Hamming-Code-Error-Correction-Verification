
#include <stdio.h>
#include <stdint.h>

#define INFO_BITS 26u
#define PARITY_BITS 5u
#define CODE_BITS 31u

//

int get_bit(uint32_t value, unsigned int position);
uint32_t toggle_bit(uint32_t value, unsigned int position);
uint32_t set_bit(uint32_t value, unsigned int position);

//

uint32_t pack_information_word(const uint8_t bits[INFO_BITS]);
int is_parity_position(unsigned int position);
uint32_t place_information_bits(uint32_t information_word);
int calculate_parity_bit(uint32_t codeword, unsigned int parity_position);

//

uint32_t encode_hamming(uint32_t information_word);

//

void build_generator_matrix(uint8_t G[INFO_BITS][CODE_BITS]);
void build_check_matrix(uint8_t H[PARITY_BITS][CODE_BITS]);
int verify_matrices(const uint8_t G[INFO_BITS][CODE_BITS], const uint8_t H[PARITY_BITS][CODE_BITS]);
int calculate_syndrome(uint32_t received_word, const uint8_t H[PARITY_BITS][CODE_BITS]);

//

int decode_hamming(uint32_t received_word, const uint8_t H[PARITY_BITS][CODE_BITS], uint32_t *decoded_information_word);