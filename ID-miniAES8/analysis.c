#include <stdio.h>
#include <stdlib.h>
#include "miniAES.h"
#include <string.h> /* memset */
#include <unistd.h> 
#define NO_OF_NIBBLES 4
#define MATRIX_SIZE 2
#define NIBBLE_SIZE 4

unsigned int Log2n(unsigned char n)
{
   return (n > 1)? 1 + Log2n(n/2): 0;
}


int main()
{

// printf("\n %d \n",Log2n(191^255));
	int n = 2; //no of nibbles in key active
	int k;
	int len = (1<<(n*NIBBLE_SIZE))/8;
    int numkeys = 1<<(n*NIBBLE_SIZE);
    
    int k1,k2,k3,k4;

	unsigned char i,j;
	unsigned char xor,t_l,t_r,k0;
	readData();

	long long count=0;
	long long totcount=0;

	for (k1 = 0; k1 <= 15; ++k1)
	{
		for (k2 = 0; k2 <= 15; ++k2)
		{
			for (k3 = 0; k3 <= 15; ++k3)
			{
				for (k4 = 0; k4 <= 15; ++k4)
					{
						count=0;
						totcount = 0;
						char key[NO_OF_NIBBLES]={k1,k2,k3,k4};
						for (i = 0; ; ++i)
						{
							char X[NO_OF_NIBBLES];
							X[0]=(i&0xF0)>>4;
							X[1]=0x09;
							X[2]=0x0A;
							X[3]=i&0x0F;
							char cipherX[NO_OF_NIBBLES];
							aesEncryption(X,cipherX,key,5,0);
							
							for (j = i+1; j!=0x00; ++j)
							{
								xor = i^j; // 0000 1010
								t_l = xor&0xF0; // 0000
								t_r = xor&0x0F; // 1010
								if(!t_l)
								{
									j=j+16-(j&0x0F)-1;

								}
								else if(!t_r)
								{
									continue;	
								}
								else if(t_l && t_r)
								{
									totcount++;
									if(totcount==8192)break;
									char Y[NO_OF_NIBBLES];
									Y[0]=(j&0xF0)>>4;
									Y[1]=0x09;
									Y[2]=0x0A;
									Y[3]=j&0x0F;
									
									char cipherY[NO_OF_NIBBLES];
									aesEncryption(Y,cipherY,key,5,0);
									int boolA = cipherY[0]==cipherX[0] && cipherY[3]==cipherX[3] && cipherY[1]!=cipherX[1] && cipherY[2]!=cipherX[2] ; 
									int boolB = cipherY[0]!=cipherX[0] && cipherY[3]!=cipherX[3] && cipherY[1]==cipherX[1] && cipherY[2]==cipherX[2] ; 
									if(boolA || boolB)
									{
										count++;
									}

								}
							}
							if(totcount==8192)break;

							if (i==0xFF)
							{
								break;
							}
						}
						printNibbles(key,sizeof(key));
						printf(" ---- %llu  -- %llu \n",totcount,count );
						// printf("\n%llu\n",count );
						
					}	
			}			
		}

	}
	// printf("%d\n",invalidCount );
	// for (k = 0; k < len; ++k){
	// 	printf("\n %d %d",(unsigned char)(keyInvalid[k]),k);
	// }
					      
	
}