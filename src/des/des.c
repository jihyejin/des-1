#include "config.h"
#include "des.h"
#include <string.h>

static char initial_permutation[] = {
    58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6,
    64, 56, 48, 40, 32, 24, 16,  8,
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7
};

static char final_permutation[] = {
    40,  8, 48, 16, 56, 24, 64, 32, 
    39,  7, 47, 15, 55, 23, 63, 31, 
    38,  6, 46, 14, 54, 22, 62, 30, 
    37,  5, 45, 13, 53, 21, 61, 29, 
    36,  4, 44, 12, 52, 20, 60, 28, 
    35,  3, 43, 11, 51, 19, 59, 27, 
    34,  2, 42, 10, 50, 18, 58, 26, 
    33,  1, 41,  9, 49, 17, 57, 25
};

static char f_expansion[] = {
    32,  1,  2,  3,  4,  5,  4,  5,
     6,  7,  8,  9,  8,  9, 10, 11,
    12, 13, 12, 13, 14, 15, 16, 17, 
    16, 17, 18, 19, 20, 21, 20, 21,
    22, 23, 24, 25, 24, 25, 26, 27,
    28, 29, 28, 29, 30, 31, 32,  1
};

static char f_permutation[] = {
    16,  7, 20, 21, 29, 12, 28, 17,
     1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9,
    19, 13, 30,  6, 22, 11,  4, 25
};

static char f_sboxes[8][64] = {{
    14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
     0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
     4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
}, {
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
     3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
     0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
}, {
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
}, {
     7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
}, {
     2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
}, {
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
     9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
}, {
     4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
}, {
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
     1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
     7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
}};

static char permuted_choice_1[] = {
    57, 49, 41, 33, 25, 17,  9,
     1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
     7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
};

static char permuted_choice_2[] = {
    14, 17, 11, 24,  1,  5,
     3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// In rounds {1, 2, 9, 16} the two halves are each rotated left by one bit.
// In all other rounds where the two halves are each rotated left by two bits.
static char round_shifts[] = {
//  1   2                           9                          16
    1,  1,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  1
};

static uint32_t permute32(uint32_t input, const size_t input_bits,
    uint8_t ptable[], const size_t output_bits)
{
    uint32_t result = 0;
    for (int i = 0; i<output_bits; i++)
    {
        result <<= 1;
        result |= (input >> (input_bits-ptable[i])) & 1;
    }
    return result;
}

static uint64_t permute64(uint64_t input, const size_t input_bits,
    uint8_t ptable[], const size_t output_bits)
{
    uint64_t result = 0;
    for (int i = 0; i<output_bits; i++)
    {
        result <<= 1;
        result |= (input >> (input_bits-ptable[i])) & 1L;
    }
    return result;
}

// derives 16 round keys (or subkeys) of 48 bits each from the original 56 bit key
static void des_key_schedule(des_t *des)
{
    // 1] apply permuted choice 1 (parity bits are removed here)
    uint64_t pc1 = permute64(des->key, 64, permuted_choice_1, 56);
    // 2] split key into 28-bit halves
    uint32_t c = (uint32_t)(pc1 >> 28 & 0x0fffffff);
    uint32_t d = (uint32_t)(pc1 & 0x0fffffff);
    // 3] calculate 16 round keys
    for (int i = 0; i<16; i++)
    {
        // rotate halves left by 1 or 2 bits
        for (int j = 0; j<round_shifts[i]; j++)
        {
            c = c << 1 & 0x0fffffff | c >> 27 & 1;
            d = d << 1 & 0x0fffffff | d >> 27 & 1;
        }
        uint64_t pc2 = (uint64_t)c << 28 | (uint64_t)d;
        int ki = des->mode=='e' ? i : 15-i;
        des->round_keys[ki] = permute64(pc2, 56, permuted_choice_2, 48);
    }
}

// f-function - main operation of DES
static uint32_t des_f(des_t *des, const uint32_t *right, int round)
{
    // 1] expand round key
    uint64_t input = permute64(*right, 32, f_expansion, 48);
    // 2] XOR with round key
    input ^= des->round_keys[round];
    // 3] apply S-box substitution (6 bit -> 4 bit)
    uint32_t output = 0;
    for (int i = 0; i<8; i++)
    {
        // 00100001 : row bits
        // 00011110 : column bits
        // 00111111 : mask = 0x3f
        // 6*i = 0, 6, ..., 42
        char chunk = (char)((input >> (42-6*i)) & 0x3f);
        char row = chunk >> 4 | chunk & 1;
        char column = chunk >> 1 & 0xf;
        output <<= 4;
        output |= f_sboxes[i][16*row+column];
    }
    // 4] permute s-box substitution output
    return permute32(output, 32, f_permutation, 32);
}

static void des_apply_round(des_t *des, uint32_t *left, uint32_t *right, int round)
{
    // 1] calculate f(R, k)
    uint32_t f = des_f(des, right, round);
    // 2] assign R to L, XOR(L, f) to R
    uint32_t temp = *right;
    *right = *left ^ f;
    *left = temp;
}

#define LITTLE_ENDIAN ('\x01\x23\x45\x67'==0x01234567)

#if LITTLE_ENDIAN
static void swap_endianness(uint64_t *val)
{
    uint64_t temp = *val;
    temp = temp << 8 & 0xFF00FF00FF00FF00ULL | temp >> 8 & 0x00FF00FF00FF00FFULL;
    temp = temp << 16 & 0xFFFF0000FFFF0000ULL | temp >> 16 & 0x0000FFFF0000FFFFULL;
    *val = temp << 32 | temp >> 32;
}
#endif

void des_init(des_t *des, uint64_t key, char mode)
{
#if LITTLE_ENDIAN
    swap_endianness(&key);
#endif
    des->key = key;
    des->mode = mode;
    des_key_schedule(des);
}

void des_deinit(des_t *des)
{ memset(des, 0, sizeof(des_t)); }

int des_verify_key(uint64_t *key, int fix)
{
    uint8_t *key_bytes = key;
    int par;
    for (int i = 0; i<8; i++)
    {
        par = 0;
        for (int j = 0; j<64; j++)
        {
            if (key_bytes[i] & 1 << j)
                par = !par;
        }
        if (!par)
        {
            if (!fix)
                return 1;
            key_bytes[i] ^= 1;
        }
    }
    return 0;
}

void des_setup(des_t *des, char mode)
{
    if (des->mode==mode)
        return;
    des->mode = mode;
    // reverse round key order
    for (int i = 0; i<8; i++)
    {
        uint64_t k = des->round_keys[i];
        des->round_keys[i] = des->round_keys[15-i];
        des->round_keys[15-i] = k;
    }
}

void des_transform_block(des_t *des, void *src, void *dst)
{
    uint64_t block = *(uint64_t*)src;
#if LITTLE_ENDIAN
    swap_endianness(&block);
#endif
    // 1] apply initial permutation
    block = permute64(block, 64, initial_permutation, 64);
    // 2] split permuted block into L and R sub-blocks
    uint32_t left = (uint32_t)(block >> 32);
    uint32_t right = (uint32_t)(block & (uint32_t)-1);
    // 3] calculate round keys - done by des_init
    // 4] apply 16 rounds
    for (int i = 0; i<16; i++)
        des_apply_round(des, &left, &right, i);
    // 5] assemble block from L and R
    block = (uint64_t)right << 32 | (uint64_t)left;
    // 6] apply final permutation
    block = permute64(block, 64, final_permutation, 64);
#if LITTLE_ENDIAN
    swap_endianness(&block);
#endif
    *(uint64_t*)dst = block;
}
