#include <stdio.h>
#include<stdlib.h>

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



void getInput(char X[4])
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

void shiftRow(char X[4])
{

}


int main()
{
	char X[4];
	char Y[4];
	
	getInput(X);
	getInput(Y);

/*
0100001111101001
1110001111101001
*/


	printf("%d\n",Y[0] );
	printf("%d\n",Y[1] );
	printf("%d\n",Y[2] );
	printf("%d\n",Y[3] );

	return 0;
}
