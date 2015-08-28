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
    
    char *keyInvalid = (char *)malloc(sizeof(char)*len);
    memset(keyInvalid,0, sizeof(char)*len);
    int invalidCount = 0;


	unsigned char i,j;
	unsigned char xor,t_l,t_r,k0;
	readData();

	long long count=0;
	char key[NO_OF_NIBBLES]={12,3,15,4};
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

					for (k = 0; k < numkeys && invalidCount<numkeys ; ++k)
				    {
				        int ind1 = k/8;
				        int indbit = k%8;
				        /*printf("\n%d %d %d %s",i,ind1,indbit,byte_to_binary(keyInvalid[ind1]) );*/
				        if(keyInvalid[ind1]&(1<<(7-indbit))){

				            //printf("\n%d ",k);
				            //invalid key
				            continue;
				        }
				        k0=(unsigned char)k;
				        char subkey0[NO_OF_NIBBLES];
				        char P[NO_OF_NIBBLES];
				        char P1[NO_OF_NIBBLES];
						subkey0[0]=(k0&0xF0)>>4;
						subkey0[1]=0x01;
						subkey0[2]=0x0B;
						subkey0[3]=k0&0x0F;
						aesEncryption(X,P,subkey0,1,1);
						aesEncryption(Y,P1,subkey0,1,1);
						// printNibbles(subkey0,sizeof(subkey0));
						// printf("cipherX: ");
						// printNibbles(P,sizeof(P));
						// printNibbles(Y,sizeof(Y));
						// printf("cipherY: ");
						// printNibbles(P1,sizeof(P1));

				        //check if key is invalid
				        if(P[0]!=P1[0]&&P[1]==P1[1]){
				        	// printf("\n -- %s original",byte_to_binary(keyInvalid[ind1]));
				            keyInvalid[ind1] = keyInvalid[ind1] | (1<<(7-indbit));
				        	// printf("\n -- %s",byte_to_binary(keyInvalid[ind1]));

				            invalidCount++;
				            // printf("\n \n invalidCount is %d ",invalidCount);

				        }
				        
    				
	    				if(invalidCount == numkeys - 1){
	    					//finding the key which is not valid
					        for (k = 0; k < len; ++k)
					        {
					            char x= keyInvalid[k]^(0XFF);
					            if(x){
					                int ind = k*8+7-(Log2n(x));
					                printf("\n \n key found partial is %x \n",ind);
					                return;
					                break;

					            }
					        }
					    }
					}

				}

			}
		}

		if (i==0xFF)
		{
			break;
		}
	}
	printf("\n%llu\n",count );
	printf("%d\n",invalidCount );
	for (k = 0; k < len; ++k){
		printf("\n %d %d",(unsigned char)(keyInvalid[k]),k);
	}
					      
	
}