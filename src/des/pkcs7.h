#pragma once
#include "config.h"
#include "common.h"

int pkcs7_pad(uint8_t *block, size_t block_size, size_t filled);
int pkcs7_depad(const uint8_t *block, size_t block_size, size_t *pad_size);
