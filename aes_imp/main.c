#include "aes_intrinsic.h"
#include<stdio.h>

int main(int argc, char *argv[])
{
    
    const int keylength = 16;
    ALIGN16 uint8_t rndkey[16 * 16] = {};
    ALIGN16 uint8_t key[keylength] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    ALIGN16 uint8_t plaintext[16] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
    ALIGN16 uint8_t ciphertext[16] = {};
    //printf("\n%lx",plaintext);
    int i;
    for(i=0;i<16;i++){
        printf("%x ",rndkey[i]);
    }
    printf("\n");
    AES128_ECB_encrypt(plaintext,key,10,ciphertext);
    for(i=0;i<16;i++){
        printf("%x ",ciphertext[i]);
    }
    return 0;
}
