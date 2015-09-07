#include <stdio.h>
#include <stdlib.h>
#include "miniAES.h"
#include <string.h> /* memset */
#include <unistd.h> 
#define NO_OF_NIBBLES 4
#define MATRIX_SIZE 2
#define NIBBLE_SIZE 4

int main()
{
	int i;
	char key[NO_OF_NIBBLES]={1,3,15,4};
	readData();
	char X[NO_OF_NIBBLES];
	X[1]=0x09;
	X[2]=0x0A;
	X[3]=0x0F;
	char cipherX[NO_OF_NIBBLES];

	char xor[NO_OF_NIBBLES];
	xor[0]=0;
	xor[1]=0;
	xor[2]=0;
	xor[3]=0;
	for(i=0;i<16;i++)
	{
		X[0]=(char)i;
		aesEncryption(X,cipherX,key,2,0);
		xor[0]^=cipherX[0];
		xor[1]^=cipherX[1];
		xor[2]^=cipherX[2];
		xor[3]^=cipherX[3];
		printNibbles(cipherX,sizeof(cipherX));
	}
	// printNibbles(xor,sizeof(xor));
	return 0;
}