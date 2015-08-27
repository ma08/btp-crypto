#include <stdio.h>

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