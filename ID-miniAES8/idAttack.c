#include <stdio.h>
#include <stdlib.h>
#include "miniAES.h"
#include <string.h> /* memset */
#include <unistd.h> 

unsigned int Log2n(unsigned char n)
{
   return (n > 1)? 1 + Log2n(n/2): 0;
}


int main(int argc, char *argv[])
{
	
	int powlimit = 27;
	if(argc>1)
		sscanf(argv[1],"%d",&powlimit);
	/*printf("\n%d",powlimit);*/
	fflush(stdout);
	int n = 2; //no of nibbles in key active
	int k;
	int len = (1<<(n*NIBBLE_SIZE))/8;
    int numkeys = 1<<(n*NIBBLE_SIZE);
    
    char *keyInvalid = (char *)malloc(sizeof(char)*len);
    memset(keyInvalid,0, sizeof(char)*len);
    int invalidCount = 0;

	int pairCount = 0;


	unsigned short i,j;
	unsigned short x,t_l,t_r,k0;
	readData();

	long long count=0;
	unsigned char key[NO_OF_NIBBLES]={0xff,3,15,0xab};
	for (i = 0; ; ++i)
	{
		if(pairCount > 1<<powlimit){
			break;
		}
		unsigned char X[NO_OF_NIBBLES];
		X[0]=(i&0xFF00)>>8;
		X[1]=0x09;
		X[2]=0x0A;
		X[3]=i&0x00FF;
		unsigned char cipherX[NO_OF_NIBBLES];
		aesEncryption(X,cipherX,key,5,0);
		
		for (j = i+1; j!=0x0000; ++j)
		{
			x = i^j; // 0000 1010
			t_l = x&0xFF00; // 0000
			t_r = x&0x00FF; // 1010
			if(!t_l)
			{
				j=j+256-(j&0x00FF)-1;

			}
			else if(!t_r)
			{
				continue;	
			}
			else if(t_l && t_r)
			{
				pairCount++;

				unsigned char Y[NO_OF_NIBBLES];
				Y[0]=(j&0xFF00)>>8;
				Y[1]=0x09;
				Y[2]=0x0A;
				Y[3]=j&0x00FF;
				
				unsigned char cipherY[NO_OF_NIBBLES];
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
				        k0=(unsigned short)k;
				        unsigned char subkey0[NO_OF_NIBBLES];
				        unsigned char P[NO_OF_NIBBLES];
				        unsigned char P1[NO_OF_NIBBLES];
						subkey0[0]=(k0&0xFF00)>>8;
						subkey0[1]=0x01;
						subkey0[2]=0x0B;
						subkey0[3]=k0&0x00FF;
						aesEncryption(X,P,subkey0,1,1);
						aesEncryption(Y,P1,subkey0,1,1);
						// printNibbles(subkey0,sizeof(subkey0));
						// printf("cipherX: ");
						// printNibbles(P,sizeof(P));
						// printNibbles(Y,sizeof(Y));
						// printf("cipherY: ");
						// printNibbles(P1,sizeof(P1));

				        //check if key is invalid
				        if((P[0]!=P1[0]&&P[1]==P1[1])||(P[0]==P1[0]&&P[1]!=P1[1])){
				        	// printf("\n -- %s original",byte_to_binary(keyInvalid[ind1]));
				            keyInvalid[ind1] = keyInvalid[ind1] | (1<<(7-indbit));
				        	// printf("\n -- %s",byte_to_binary(keyInvalid[ind1]));

				            invalidCount++;
							/*printf("\n \n invalidCount is %d ",invalidCount);*/

				        }
				        
    				
	    				if(invalidCount == numkeys - 1){
	    					//finding the key which is not valid
					        for (k = 0; k < len; ++k)
					        {
					            unsigned char x= keyInvalid[k]^(0xFF);
					            if(x){
					                int ind = k*8+7-(Log2n(x));
									/*printf("\n \n key found partial is %x \n",ind);*/
									printf("%d %d %d\n",powlimit,pairCount,invalidCount );
					                return 0;
					            }
					        }
					    }
					}
				}
			}
		}
		if (i==0xFFFF)
		{
			break;
		}
	}
	printf("%d %d %d\n",powlimit,pairCount,invalidCount );
	fflush(stdout);
	/*printf("\n%llu\n",count );*/
}
