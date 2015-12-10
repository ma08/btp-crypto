#include "aes.h"
#include<stdio.h>
#include<stdint.h>
int main(int argc, char *argv[])
{

    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t in[]  = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    uint8_t out[] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
    uint8_t buffer[16];
    AES128_ECB_encrypt(in, key, 10, buffer);
    int i;
    for (i = 0; i < 16; ++i)
    {
        printf("%x ",buffer[i]);
    }
    printf("\n");
    for (i = 0; i < 16; ++i)
    {
        printf("%x ",out[i]);
    }
    printf("\n\n");
    AES128_ECB_decrypt(out, key, 10, buffer);
    for (i = 0; i < 16; ++i)
    {
        printf("%x ",buffer[i]);
    }
    printf("\n");
    for (i = 0; i < 16; ++i)
    {
        printf("%x ",in[i]);
    }
    return 0;
}