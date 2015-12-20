#pragma once
#include "config.h"
#include "common.h"

// key size = 56 bits
// block size = 64 bits
typedef struct
{
    uint64_t key;
    uint64_t round_keys[16];
    char mode; // 'e' or 'd'
} des_t;

void des_init(des_t *des, uint64_t key, char mode);
void des_deinit(des_t *des);
int des_verify_key(uint64_t *key, int fix);
void des_setup(des_t *des, char mode);
void des_transform_block(des_t *des, void *src, void *dst);
