#include<stdint.h>
#define ALIGN16 __attribute__ ((aligned(16)))
void AES128_ECB_encrypt(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext);
void AES256_ECB_encrypt(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext);
void AES256_ECB_encrypt_inclastmix(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext);
