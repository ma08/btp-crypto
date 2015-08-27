#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

char *sbox;

char **mixmatrix;

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
	char c[8],d[8];
	const size_t line_size = 300;
	char* line = (char *)malloc(line_size);
	fp = fopen( "sbox.txt" , "r");
	if(fp == NULL)
		return;
	while (fgets(line, line_size, fp) != NULL)  {
		sscanf(line,"%s %s",c,d);
		int index = readNibble(c);
		char out = readNibble(d);
		sbox[index] = out;
		/*printf("%d %d\n",index,(int)(sbox[index]));*/
	}
	free(line);
	fclose(fp);
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

void multiplyMixMatrix(char* mix,char *state,int size){
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
				sum = sum ^ multiply(mix[i*size+k],temp[k*size+j]);
			}
			state[i*size+j]=sum;
		}
	}
}

void keySchedule(char* prevSubKey, int round){
	int size = 4;
	char temp[4];
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
		printf("%d\n",X[i] );
	}
}


int main()
{
	char X[4];
	char Y[4];
	// char Y[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	
	getInput(X);
	getInput(Y);
	// printNibbles(Y,sizeof(Y));
	shiftRow(Y,sqrt(sizeof(Y)));
	// printNibbles(Y,sizeof(Y));


/*
0100001111101001
1110001111101001

*/


	// printf("%d\n",Y[0] );
	// printf("%d\n",Y[1] );
	// printf("%d\n",Y[2] );
	// printf("%d\n",Y[3] );

	return 0;
}
