#ifndef MINIAES_H_
#define MINIAES_H_
#define NO_OF_NIBBLES 4
#define MATRIX_SIZE 2
#define NIBBLE_SIZE 8


void aesEncryption(unsigned char plainText[],unsigned char cipherText[],const unsigned char key[],int rounds,int idFlag);
void printNibbles(unsigned char *X,int size);
void readData();
const char *byte_to_binary(unsigned char x);


#endif //MINIAES_H_
