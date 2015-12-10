#ifndef MINIAES_H_
#define MINIAES_H_
void aesEncryption(char plainText[],char cipherText[],const char key[],int rounds,int idFlag);
void printNibbles(char *X,int size);
void readData();
char multiply(char a, char b);

const char *byte_to_binary(char x);


#endif //MINIAES_H_