#include "aes.h"
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<bits/stdc++.h>

unsigned int keyInvalid[1<<27];

typedef uint8_t state_t[4][4];

void PrintState(state_t* s){
    int i,j;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
            printf("%x ",(*s)[j][i]);
        printf("\n");
    }
    printf("\n");
}
using namespace std;
unsigned int bufToInt(uint8_t *buf){
        unsigned int a;
        a=0;
        a=a|(buf[0]<<24);
        a=a|(buf[5]<<16);
        a=a|(buf[10]<<8);
        a=a|(buf[15]);
        return a;
}
void intToBuf(uint8_t *buf, unsigned int a){
        int i;
        long y;
        for (i = 0; i < 16; i++) 
            if(i%5!=0)
                buf[i]=0;
        for (i = 0; i < 4; ++i){
            y = 0xFF<<(i*8);
            buf[i*5] = (a&y)>>(i*8);
        }
}

//0 indexed
int nthbit(unsigned int n){

	int ind1 = n/32;
	int indbit = n%32;
	return keyInvalid[ind1]&((unsigned int)1<<(32-indbit-1));
}

void setnthbit(int n){
	int ind1 = n/32;
	int indbit = n%32;
	unsigned int x =(unsigned int)1<<31;
	//printf("\n%u\n",x);
	x = x>>(indbit);
	//printf("\n%u\n",x);
	keyInvalid[ind1]=keyInvalid[ind1]|x;
}

map<unsigned int,std::vector<unsigned int> >hashTable;


int main(int argc, char *argv[])
{
    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t *c1,*out;
    unsigned int hashkey;
    c1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    out = (uint8_t *)malloc(sizeof(uint8_t)*16);
    state_t* s1 = (state_t*)c1;
    int  a, b, c, d;
    for (a = 0; a < 1<<8; a++)
        for (b = 0; b < 1<<8; ++b)
            for (c = 0; c < 1<<8; ++c)
                for (d = 0; d < 1<<8; ++d){
                    (*s1)[0][0]=a;
                    (*s1)[0][1]=b;
                    (*s1)[0][2]=c;
                    (*s1)[0][3]=d;
                    AES128_ECB_encrypt(c1, key, 5, out);
                    hashkey = bufToInt(out);
                    if(hashTable.find(hashkey)==hashTable.end()){
                        vector<unsigned int> temp;
                        hashTable[hashkey] = temp;
                    }
                    hashTable[hashkey].push_back(bufToInt(c1));
                }
 
    return 0;
}
