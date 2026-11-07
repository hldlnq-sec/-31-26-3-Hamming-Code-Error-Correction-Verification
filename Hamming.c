#include <corecrt.h>
#include <stdint.h>

#define INFO_BITS 26u
#define PARITY_BITS 5u
#define CODE_BITS 31u

//получение значения конкретного бита
int get_bit(uint32_t value, unsigned int position)
{
    if (position >= 32u)
    {
        return -1;
    }

    return (int)((value >> position) & 1u);
}

//ивертирование бита
uint32_t toggle_bit(uint32_t value, unsigned int position)
{
    if (position >= 32u)
    {
        return value;
    }

    return value ^ (UINT32_C(1) << position);
}

//
uint32_t set_bit(uint32_t value, unsigned int position)
{
    if (position >= 32u)
    {
        return value;
    }

    uint32_t mask = UINT32_C(1) << position;

    return value | mask;
}

//преобразование 
uint32_t pack_information_word(const uint8_t bits[INFO_BITS])
{
    uint32_t information_word = 0;

    for (unsigned int i = 0; i < INFO_BITS; i++)
    {
        if (bits[i] == 1)
        {
            information_word = set_bit(information_word, i);
        }
    }

    return information_word;
}

//проверка является ли позиция проверочной
int is_parity_position(unsigned int position)
{
    return position != 0u && position <= CODE_BITS && (position & (position - 1u)) == 0u;
}

//создание кодового слова длиной 31 бит
uint32_t place_information_bits(uint32_t information_word)
{
    uint32_t codeword = 0;
    unsigned int data_index = 0;

    for (unsigned int position = 1; position <= CODE_BITS; position++)
    {
        if (is_parity_position(position))
        {
            continue;
        }

        if (get_bit(information_word, data_index) == 1)
        {
            codeword = set_bit(codeword, position - 1);
        }

        data_index++;
    }

    return codeword;
}

//вычисление значений для проверочных битов
int calculate_parity_bit(uint32_t codeword, unsigned int parity_position)
{
    if (!(is_parity_position(parity_position)) || (codeword >> CODE_BITS) != 0u) return -1; //проверяю чтобы старший бит не был занят т.к. у нас 31-битное слово

    int parity = 0;

    for (unsigned int position = 1; position <= CODE_BITS; position++)
    {
        if ( (position & parity_position) != 0 && position != parity_position)
        {
            int bit = get_bit(codeword, position - 1u);

            parity ^= bit;
        }
    }

    return parity;
}

//кодирование Хэмминга
uint32_t encode_hamming(uint32_t information_word)
{
    uint32_t codeword = place_information_bits(information_word);

    for (unsigned int parity_position = 1u; parity_position <= CODE_BITS; parity_position *= 2u)
    {
        int parity = calculate_parity_bit(codeword, parity_position);

        if (parity == 1) codeword = set_bit(codeword, parity_position - 1u);
    }

    return codeword;
}

//создание порождающей матрицы
void build_generator_matrix(uint8_t G[INFO_BITS][CODE_BITS])
{
    for (unsigned int row = 0; row < INFO_BITS; row++)
    {
        uint32_t basis_word = UINT32_C(1) << row; //создаём базисное информационное слово

        uint32_t encoded_word = encode_hamming(basis_word); //кодируем базисное слово

        for (unsigned int col = 0; col < CODE_BITS; col++) //перебираем все столбцы текущей строки
        {
            G[row][col] = get_bit(encoded_word, col); //извлекаем бит кодового слова и записываем его в соответствующую ячейку
        }
    }
}

//построение проверочной матрицы
void build_check_matrix(uint8_t H[PARITY_BITS][CODE_BITS])
{
    for (unsigned int col = 0; col < CODE_BITS; col++)
    {
        unsigned int position = col + 1u;

        for (unsigned int row = 0; row < PARITY_BITS; row++)
        {
            H[row][col] = (position >> row) & 1u;
        }
    }
}

//проверка G * H^T = 0
int verify_matrices(const uint8_t G[INFO_BITS][CODE_BITS], const uint8_t H[PARITY_BITS][CODE_BITS])
{
    for (unsigned int i = 0; i < INFO_BITS; i++)
    {
        for (unsigned int j = 0; j < PARITY_BITS; j++)
        {
            uint8_t sum = 0u;

            for (unsigned int k = 0; k < CODE_BITS; k++)
            {
                sum ^= (G[i][k] & H[j][k]);
            }

            if (sum != 0u) return 0;
        }
    }

    return 1;
}

//вычисление синдрома
int calculate_syndrome(uint32_t received_word, const uint8_t H[PARITY_BITS][CODE_BITS])
{
    if ((received_word >> CODE_BITS) != 0u)
    {
        return -1;
    }

    uint32_t syndrome = 0u;

    for (unsigned int row = 0; row < PARITY_BITS; row++)
    {
        uint8_t syndrome_bit = 0u;

        for (unsigned int col = 0; col < CODE_BITS; col++)
        {
            uint8_t received_bit = get_bit(received_word, col);

            syndrome_bit ^= (received_bit & H[row][col]);
        }

        syndrome |= ((uint32_t)syndrome_bit << row);
    }

    return (int)syndrome;
}

//декодирование
int decode_hamming(uint32_t received_word, const uint8_t H[PARITY_BITS][CODE_BITS], uint32_t *decoded_information_word)
{
    if (decoded_information_word == NULL) return -1;

    int syndrome = calculate_syndrome(received_word, H);
    if (syndrome == -1) return -1;

    int status_syndrome = 0;
    uint32_t corrected_word = received_word;

    if (syndrome > 0)
    {
        corrected_word = toggle_bit(corrected_word, syndrome - 1);
        status_syndrome = 1;
    }

    uint8_t information_bits[INFO_BITS];
    unsigned int info_index = 0u;

    for (unsigned int position = 1u; position <= CODE_BITS; position++)
    {
        if (!is_parity_position(position))
        {
            information_bits[info_index] = (uint8_t)get_bit(corrected_word, position - 1u);

            info_index++;
        }
    }

    *decoded_information_word = pack_information_word(information_bits);

    return status_syndrome;
}