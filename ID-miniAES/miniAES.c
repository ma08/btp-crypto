#include <stdio.h>
#include <math.h>

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