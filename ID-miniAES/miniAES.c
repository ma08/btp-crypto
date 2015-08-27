#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


#define NO_OF_NIBBLES 4
#define MATRIX_SIZE 2
#define NIBBLE_SIZE 4

char *sbox;
char *inverseSbox;

char mixmatrix[MATRIX_SIZE][MATRIX_SIZE];

const char *byte_to_binary(char x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

char multiply(char a, char b){
	char x = 3;
	char output = 0;
	int i,size=4;
	if(1 & b)
		output = a;
	for (i = 1; i < size; ++i)
	{
		if((1<<(size-1))&a){
			a = a << 1;  
			a = a ^ x;
		}else
			a = a << 1;  
		if((1<<(i))&b){
			/*printf("%d inside %s\n",i,byte_to_binary(output));*/
			output = output ^ a;
		}
		/*printf("%d %s ",i,byte_to_binary(a));*/
		/*printf("%s\n",byte_to_binary(output));*/
	}
	return output&((1<<size)-1);
}

char readNibble(char *s){
	char output = 0;
	int i;
	for ( i = 0; i < 4; ++i )
		output |= (s[i] == '1') << (3 - i);
	return output;
}

void readSbox(const char* filename, char *sbox){
	FILE * fp;
	fp = fopen( filename , "r");
	if(fp == NULL)
		return;
	int a,b,i;
	for (i = 0; i < (1 << NIBBLE_SIZE); ++i)
	{
		fscanf(fp,"%x %x",&a,&b);
		sbox[a]=(char)b;
	}
	fclose(fp);

	for (i = 0; i < (1<<NIBBLE_SIZE); ++i)
	{
		inverseSbox[sbox[i]]=i;
	}
}

void readMixMatrix(const char* filename, char *mixMatrix,int size){
	FILE * fp;
	fp = fopen( filename , "r");
	if(fp == NULL)
		return;
	int x;
	int i=0,j=0;
	for (i = 0; i < size; ++i)
	{
		for (j = 0; j < size; ++j)
		{
			fscanf(fp,"%x",&x);
			mixMatrix[i*size+j]=(char)x;
		}
	}
	fclose(fp);
}

void mixColumn(char *state,int size){
	char* mix = &mixmatrix[0][0];
	char *temp = (char *)malloc(sizeof(char)*(size*size));
	int i,j,k;
	for (i = 0; i < size; ++i)
	{
		for (j = 0; j < size; ++j)
			temp[i*size+j] = state[i*size+j];
	}
	char sum = 0;
	for (i = 0; i < size; ++i)
	{
		for (j = 0; j < size; ++j)
		{
			sum = 0;
			for (k = 0; k < size; ++k)
			{
				sum = sum ^ multiply(mix[i*size+k],temp[j*size+k]);
			}
			state[j*size+i]=sum;
		}
	}
}

void keySchedule(char* prevSubKey, int round){
	int size = 4;
	char temp[NO_OF_NIBBLES];
	int i;
	if(round==0)
		return;
	for (i = 0; i < size; ++i){
		temp[i] = prevSubKey[i];
		
	}
	prevSubKey[0] =( temp[0])^(sbox[(int)temp[size-1]])^((char)round);
	for (i = 1; i < size; ++i)
	{
		prevSubKey[i] = temp[i] ^ prevSubKey[i-1];
	}
	return;
}



void getInput(char *X)
{
    int i;
	int j=4;
	char readInput[17];
	if (fgets(readInput, sizeof readInput, stdin)) {
		while(j--)
		{
			X[j]=0;
			for ( i = 4*j; i < 4*j+4; ++i )
		    	X[j] |= (readInput[i] == '1') << (4*j+3 - i);
		}
    	getchar();

    }
    else
    	return ;
	
}


void reverseArray(char arr[], int start, int end, int rowNo, int size)
{
	char temp;
	while(start < end)
	{
		temp = arr[start*size+rowNo];   
		arr[start*size+rowNo] = arr[end*size+rowNo];
		arr[end*size+rowNo] = temp;
		start++;
		end--;
	}   
}
void rotateArray(char arr[],int d,int size,int rowNo)
{
	reverseArray(arr,0,d-1,rowNo,size);
	reverseArray(arr,d,size-1,rowNo,size);
	reverseArray(arr,0,size-1,rowNo,size);
} 

void shiftRow(char *X,int size)
{
	int i;
	for (i = 1; i < size; ++i)
	{
		rotateArray(X,i,size,i);
	}	
}
void printNibbles(char *X,int size)
{
	int i;
	for (i = 0; i < size; ++i)
	{
		printf("%d ",X[i] );
	}
	printf("\n");
}
void keyAddition(char temp[],char subkey[])
{
	int i;
	for (i = 0; i < NO_OF_NIBBLES; ++i)
	{
		temp[i]^=subkey[i];
	}
}

void copy(char out[],const char in[])
{
	int i;
	for (i = 0; i < NO_OF_NIBBLES; ++i)
	{
		out[i] = in[i];
	}
}

void nibbleSub(char cipherText[])
{
	int i;
	for (i = 0; i < NO_OF_NIBBLES; ++i)
	{
		cipherText[i] = sbox[cipherText[i]];
	}
}

void aesEncryption(char plainText[],char cipherText[],const char key[],int rounds)
{
	int i;
	char tempKey[NO_OF_NIBBLES];
	copy(tempKey,key);
	copy(cipherText,plainText);
	keyAddition(cipherText,tempKey);

	for (i = 1; i <= rounds; ++i)
	{
		nibbleSub(cipherText);
		shiftRow(cipherText,MATRIX_SIZE);
		if(i!=rounds)
			mixColumn(cipherText,MATRIX_SIZE);
		keySchedule(tempKey,i);
		keyAddition(cipherText,tempKey);
	}
}

void aesDecryption(char plainText[],char cipherText[],const char key[],int rounds)
{
	int i;

	char **subkeys = malloc(sizeof(char*)*(rounds+1));
	for (i = 0; i <= rounds; ++i)
	{
		subkeys[i] = malloc(sizeof(char)*NO_OF_NIBBLES);
	}
	char tempKey[NO_OF_NIBBLES];
	copy(tempKey,key);

	for (i = 0; i < rounds+1; ++i)
	{
		keySchedule(tempKey,i);
		copy(subkeys[i],tempKey);
	}
	char *temp = sbox;
	sbox = inverseSbox;

	copy(plainText,cipherText);
	keyAddition(plainText,subkeys[rounds]);
	for (i = rounds; i >= 1; --i)
	{
		if(i!=rounds)
			mixColumn(plainText,MATRIX_SIZE);
		nibbleSub(plainText);
		shiftRow(plainText,MATRIX_SIZE);
		keyAddition(plainText,subkeys[i-1]);
	}


	sbox = temp;
}
int main()
{
	char X[NO_OF_NIBBLES];
	char cipherX[NO_OF_NIBBLES];
	char Y[NO_OF_NIBBLES];
	char key[NO_OF_NIBBLES]={12,3,15,0};

	sbox = malloc(sizeof(char)*(1<<NIBBLE_SIZE));
	inverseSbox = malloc(sizeof(char)*(1<<NIBBLE_SIZE));



	readMixMatrix("mixmatrix.txt",&mixmatrix[0][0],MATRIX_SIZE);
	readSbox("sbox.txt",sbox);
	getInput(X);
	printNibbles(X,sizeof(X));
	aesDecryption(cipherX,X,key,2);
	// aesEncryption(X,cipherX,key,2);
	printNibbles(cipherX,sizeof(cipherX));


/*
0100001111101001
1110001111101001

0101111101101100


1001110001100011

*/


	return 0;
}
