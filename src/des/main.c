#include "config.h"
#include "common.h"
#include "des.h"
#include "pkcs7.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void print_usage()
{
    const char *usage_str = "usage: des {encrypt|decrypt} "
        "[-f|--fix-parity] <key> <source file> <destination file>";
    puts(usage_str);
}

static int fsize(const char *path, size_t *size)
{
    struct stat buffer;    
    if (stat(path, &buffer))
        return 1;
    *size = buffer.st_size;
    return 0;
}

static int parse_key(const char *str, uint64_t *key)
{
    char *end;
    errno = 0;
    uint64_t result = strtoull(str, &end, 16);
    if (!result && end==str)
        return 1; // str was not a number
    if (result==ULLONG_MAX && errno)
        return 1; // the value of str doesn't fit in uint64_t
    if (*end)
        return 1; // str began with a number but has junk left over at the end
    *key = result;
    return 0;
}

typedef struct
{
    char *key_str;
    int fix_parity;
    char des_mode;
    char *src_path;
    char *dst_path;
} args_t;

void args_init(args_t *args)
{ memset(args, 0, sizeof(args_t)); }

typedef struct
{
    char *name_s;
    char *name_l;
} opt_t;

const opt_t opts[] = {
    {"-f", "--fix-parity"},
    {"", "encrypt"},
    {"", "decrypt"},
    {NULL, NULL}
};

#define OPT_FIX_PARITY 0
#define OPT_ENCRYPT 1
#define OPT_DECRYPT 2

static int is_option(const char *s, const opt_t *opt)
{ return !strcmp(s, opt->name_s) || !strcmp(s, opt->name_l); }

static int find_option(const char *s, const opt_t *opt)
{
    for (int i = 0; opt->name_l; i++)
    {
        if (is_option(s, opt))
            return i;
        opt++;
    }
    return -1;
}

int args_parse(args_t *args, int argc, char *argv[])
{
    int i = 1;
    int mode = find_option(argv[i++], opts);
    if (mode==OPT_ENCRYPT)
        args->des_mode = 'e';
    else if (mode==OPT_DECRYPT)
        args->des_mode = 'd';
    else
        return 1;
    if (argc==6)
    {
        if (find_option(argv[i++], opts)!=OPT_FIX_PARITY)
            return 1;
        args->fix_parity = 1;
    }
    args->key_str = argv[i++];
    args->src_path = argv[i++];
    args->dst_path = argv[i++];
    return 0;
}

#define BLOCK_SIZE 8

int main(int argc, char *argv[])
{
    if (argc!=5 && argc!=6)
    {
        print_usage();
        return 1;
    }
    args_t args;
    args_init(&args);
    if (args_parse(&args, argc, argv))
    {
        print_usage();
        return 1;
    }
    uint64_t key = 0;
    if (parse_key(args.key_str, &key))
    {
        puts("invalid key.");
        return 1;
    }
    if (des_verify_key(&key, args.fix_parity))
    {
        puts("key parity check failed.");
        return 1;
    }
    FILE *src = fopen(args.src_path, "rb");
    if (!src)
    {
        puts("can't open source file.");
        return 1;
    }
    size_t src_size;
    if (fsize(args.src_path, &src_size))
    {
        printf("can't stat %s.\n", args.src_path);
        return 1;
    }
    if (args.des_mode=='d' && src_size%BLOCK_SIZE)
    {
        puts("source file length must be a multiple of "
            "block size (8 bytes).");
        return 1;
    }
    FILE *dst = fopen(args.dst_path, "wb+");
    if (!dst)
    {
        puts("can't open destination file.");
        return 1;
    }
    size_t blocks_done = 0;
    size_t plain_blocks = src_size/BLOCK_SIZE;
    if (args.des_mode=='d')
        plain_blocks--;
    des_t des;
    des_init(&des, key, args.des_mode);
    // decrypting: transform plain_blocks-1, apply special case for last block
    // encrypting: transform plain_blocks, apply special case for the rest bytes    
    uint8_t buf[CONFIG_TRANSFORM_BUFFER_SIZE];
    uint8_t *buf_ptr = buf;
    size_t remain = 0;
    while (1)
    {
        size_t bytes_read = fread(buf+remain, 1,
            CONFIG_TRANSFORM_BUFFER_SIZE-remain, src);
        if (!bytes_read) // nothing remained, proceed with the last block
        {
            if (buf_ptr==buf+CONFIG_TRANSFORM_BUFFER_SIZE)
                buf_ptr = buf;
            break;
        }
        bytes_read += remain;
        buf_ptr = buf;
        size_t blocks_read = bytes_read/BLOCK_SIZE;
        remain = bytes_read%BLOCK_SIZE;
        size_t stage_block_lim = min(blocks_done+blocks_read, plain_blocks);
        size_t blocks_staged = stage_block_lim-blocks_done;
        for (; blocks_done<stage_block_lim; blocks_done++)
        {
            des_transform_block(&des, buf_ptr, buf_ptr);
            buf_ptr += BLOCK_SIZE;
        }
        fwrite(buf, 1, blocks_staged*BLOCK_SIZE, dst);
        if (remain)
        {
            memmove(buf, buf_ptr, remain);
            buf_ptr = buf;
        }
    }
    // process last block
    if (args.des_mode=='d')
    {
        des_transform_block(&des, buf_ptr, buf_ptr);
        size_t padding = 0;
        if (pkcs7_depad(buf_ptr, BLOCK_SIZE, &padding))
        {
            puts("padding is invalid and cannot be removed.");
            return 1;
        }
        remain = BLOCK_SIZE-padding;
    }
    else
    {
        if (pkcs7_pad(buf_ptr, BLOCK_SIZE, remain))
        {
            puts("cannot apply padding.");
            return 1;
        }
        remain = BLOCK_SIZE;
        des_transform_block(&des, buf_ptr, buf_ptr);
    }    
    fwrite(buf_ptr, 1, remain, dst);
    fclose(src);
    fclose(dst);
    return 0;
}
