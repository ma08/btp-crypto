#include <wmmintrin.h>
#include<stdint.h>
#include<stdio.h>
#include "aes_intrinsic.h"
#define ALIGN16 __attribute__ ((aligned(16)))

//Copied from INTEL
inline __m128i AES_128_ASSIST (__m128i temp1, __m128i temp2)
{
    __m128i temp3;
    temp2 = _mm_shuffle_epi32 (temp2 ,0xff);
    temp3 = _mm_slli_si128 (temp1, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp1 = _mm_xor_si128 (temp1, temp2);
    return temp1;
}

void AES_128_Key_Expansion (const unsigned char *userkey,
        unsigned char *key)
{
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1 ,0x1);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x2);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x4);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x8);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x10);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x20);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x40);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x80);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x36);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[10] = temp1;
}

inline void KEY_256_ASSIST_1(__m128i* temp1, __m128i * temp2)
{
    __m128i temp4;
    *temp2 = _mm_shuffle_epi32(*temp2, 0xff);
    temp4 = _mm_slli_si128 (*temp1, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    *temp1 = _mm_xor_si128 (*temp1, *temp2);
}
inline void KEY_256_ASSIST_2(__m128i* temp1, __m128i * temp3)
{
    __m128i temp2,temp4;
    temp4 = _mm_aeskeygenassist_si128 (*temp1, 0x0);
    temp2 = _mm_shuffle_epi32(temp4, 0xaa);
    temp4 = _mm_slli_si128 (*temp3, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    *temp3 = _mm_xor_si128 (*temp3, temp2);
}
void AES_256_Key_Expansion (const unsigned char *userkey,
        unsigned char *key)
{
    __m128i temp1, temp2, temp3;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    temp3 = _mm_loadu_si128((__m128i*)(userkey+16));
    Key_Schedule[0] = temp1;
    Key_Schedule[1] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x01);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[2]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[3]=temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x02);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[4]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[5]=temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x04);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[6]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[7]=temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3,0x08);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[8]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[9]=temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3,0x10);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[10]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[11]=temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3,0x20);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[12]=temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[13]=temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3,0x40);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[14]=temp1;
}
//http://tab.snarc.org/posts/technical/2012-04-12-aes-intrinsics.html
//n is number of rounds 128 - 10 256 - 14
void AES128_ECB_encrypt(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext){

    ALIGN16 uint8_t rndkey[16 * 16] = {};
    AES_128_Key_Expansion(key,rndkey);

    __m128i m = _mm_load_si128((const __m128i *) plaintext);
    __m128i *Key_Schedule = (__m128i*)rndkey;

    /* first xor the loaded message with k0, which is the AES key supplied */
    m = _mm_xor_si128(m, Key_Schedule[0]);
    int i;
    for(i=1;i<n;i++){
        m = _mm_aesenc_si128(m, Key_Schedule[i]);
    }
    m = _mm_aesenclast_si128(m, Key_Schedule[n]);
    _mm_store_si128((__m128i *) ciphertext, m);
}

void AES256_ECB_encrypt(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext){

    ALIGN16 uint8_t rndkey[16 * 16] = {};
    AES_256_Key_Expansion(key,rndkey);

    __m128i m = _mm_load_si128((const __m128i *) plaintext);
    __m128i *Key_Schedule = (__m128i*)rndkey;

    /*int i,j;
    for (i = 0; i < 5; ++i)
    {
        printf("\n");
        for (j = 0; j < 16; ++j)
        {
            printf("%x ",rndkey[i*16+j]);
        }
        printf("\n");
    }*/


    /* first xor the loaded message with k0, which is the AES key supplied */
    m = _mm_xor_si128(m, Key_Schedule[0]);
    for(int i=1;i<n;i++){
        m = _mm_aesenc_si128(m, Key_Schedule[i]);
    }
    m = _mm_aesenclast_si128(m, Key_Schedule[n]);
    _mm_store_si128((__m128i *) ciphertext, m);
}



void AES256_ECB_encrypt_inclastmix(const unsigned char* plaintext, const unsigned char* key, int n, unsigned char* ciphertext){

    ALIGN16 uint8_t rndkey[16 * 16] = {};
    AES_256_Key_Expansion(key,rndkey);

    __m128i m = _mm_load_si128((const __m128i *) plaintext);
    __m128i *Key_Schedule = (__m128i*)rndkey;

    /*int i,j;
    for (i = 0; i < 5; ++i)
    {
        printf("\n");
        for (j = 0; j < 16; ++j)
        {
            printf("%x ",rndkey[i*16+j]);
        }
        printf("\n");
    }*/


    /* first xor the loaded message with k0, which is the AES key supplied */
    m = _mm_xor_si128(m, Key_Schedule[0]);
    for(int i=1;i<=n;i++){
        m = _mm_aesenc_si128(m, Key_Schedule[i]);
    }
    /*m = _mm_aesenclast_si128(m, Key_Schedule[n]);*/
    _mm_store_si128((__m128i *) ciphertext, m);
}














