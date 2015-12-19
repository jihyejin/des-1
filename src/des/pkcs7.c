#include "config.h"
#include "pkcs7.h"

int pkcs7_pad(uint8_t *block, size_t block_size, size_t filled)
{
    if (block_size>256)
        return 1;
    // fill each byte value with the number of bytes padded
    for (size_t i = filled; i<block_size; i++)
        block[i] = (uint8_t)(block_size-filled);
    return 0;
}

int pkcs7_depad(const uint8_t *block, size_t block_size, size_t *pad_size)
{
    size_t sz = block[block_size-1];
    if (sz>block_size)
        return 1;
    for (size_t i = 1; i<=sz; i++)
    {
        if (block[block_size-i]!=sz)
            return 1;
    }
    *pad_size = sz;
    return 0;
}
