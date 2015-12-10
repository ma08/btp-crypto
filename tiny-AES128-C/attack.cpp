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

void PreComputation(){
    int a1, a2, b, c, d;
    long long count=0;
    for (a1 = (1<<8)-1; a1 >= 0; a1--)
        for (a2 = a1-1; a2 >= 0; a2--)
            for (b = 0; b < 1<<8; ++b)
                for (c = 0; c < 1<<8; ++c)
                    for (d = 0; d < 1<<8; ++d){
                        uint8_t *c1;
                        uint8_t *c2;
                        c1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
                        c2 = (uint8_t *)malloc(sizeof(uint8_t)*16);
                        state_t* s1 = (state_t*)c1;
                        state_t* s2 = (state_t*)c2;
                        (*s1)[0][0]=a1;
                        (*s2)[0][0]=a2;
                        (*s2)[0][1]=(*s1)[0][1]=b;
                        (*s2)[0][2]=(*s1)[0][2]=c;
                        (*s2)[0][3]=(*s1)[0][3]=d;
                        int i;
                        for (i = 4; i < 16; ++i)
                        {
                            c1[i]=c2[i]=0;
                        }
						PrintState(s1);
						PrintState(s2);
                        InvCipherSinglePreComp(c1);
                        InvCipherSinglePreComp(c2);
                        count++;
                        if(count==(((long)1)<<32)){
                            printf("%lld",count);
                            return;
                        }
						PrintState(s1);
						PrintState(s2);
						return;

                        /*printf("%d %d %d %d",(*s1)[0][0]^(*s2)[0][0],(*s1)[1][1]^(*s2)[1][1],(*s1)[2][2]^(*s2)[2][2],(*s1)[3][3]^(*s2)[3][3]);*/
                        /*return;*/
                    }
}
stringstream ss;
map<unsigned int,vector<unsigned int> > desiredPairsMap;
map<int,vector<unsigned int> > hashTable;
void DesiredPairs(uint8_t* key){

	long a,b,y;
	int i;
	long long count=0;
	
    uint8_t buffer1[16];
    uint8_t buffer2[16];
    uint8_t *c1;
	uint8_t *c2;
	c2 = (uint8_t *)malloc(sizeof(uint8_t)*16);
	c1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
	// state_t* s1 = (state_t*)c1;
	// state_t* s2 = (state_t*)c2;
	state_t* sb1 = (state_t*)buffer1;
	state_t* sb2 = (state_t*)buffer2;
	for (a = 0; a < (long)1<<32 ; ++a)
	{
		
		for (i = 0; i < 16; i++) 
			if(i%5!=0)
				c1[i]=c2[i]=0;
	
		for (i = 0; i < 4; ++i){
				y = 0xFF<<(i*8);
				c1[i*5] = (a&y)>>(i*8);
		}
		AES128_ECB_encrypt(c1, key, 5, buffer1);
		
		unsigned int hashKey=0;
		hashKey=hashKey|(buffer1[0]<<24);
		hashKey=hashKey|(buffer1[5]<<16);
		hashKey=hashKey|(buffer1[10]<<8);
		hashKey=hashKey|(buffer1[15]);
		//printf("hashed key = %x\n",hashKey );
		//printf("buffer  = %x %x %x %x\n",buffer1[0],buffer1[1],buffer1[2],buffer1[3] );
		
		if (desiredPairsMap.find(hashKey) == desiredPairsMap.end())
		{
			vector<unsigned int> temp;
			desiredPairsMap[hashKey] = temp;
		}
		else
		{
			//cout<<"hurray "<<key<<endl;
			//PrintState(sb1);
		}
		desiredPairsMap[hashKey].push_back(bufToInt(c1));
		count++;
		if(count%10000000==0)
		{
			//printf("\n%lld\n",count);
			//cout<<ss.str()<<endl;
			//printf("%x\n",buffer1[0]);
			
			typedef map<int,vector<uint8_t *> >::iterator itType;
			// int lel = 0;
			/*for(itType it = desiredPairsMap.begin();it!=desiredPairsMap.end();it++)
			{
				// lel++;
				//if(lel==10)break;
				if((it->second).size()>=2)
				{
					cout<<"--------"<<endl;
					printf("%x\n",it->first);
					for(int lol=0;lol<(it->second).size();lol++)
					{
						PrintState((state_t*)(it->second)[lol]);
						printf("`````````\n");
						AES128_ECB_encrypt((it->second)[lol], key, 5, buffer2);
						PrintState(sb2);
						printf("###########\n");
					}
					cout<<"====="<<endl<<endl;
				}
			}*/
			cout << "mymap.size() is " << desiredPairsMap.size() << '\n';
			break;
		}
		/*for (b = a+1; b < (long)1<<32 ; ++b){
			for (i = 0; i < 4; ++i)
			{
				y = 0xFF<<(i*8);
				c2[i*5] = (b&y)>>(i*8);
			}
			AES128_ECB_encrypt(c2, key, 5, buffer2);

			//desired
			//if(buffer1[0]==buffer2[0]&&buffer1[5]==buffer2[5]&&buffer1[10]==buffer2[10]&&buffer1[15]==buffer2[15]){
			count++;
			if(count%1000000==0)
				printf("\n%lld\n",count);
			if(buffer1[0]==buffer2[0]&&buffer1[5]==buffer2[5]&&buffer1[10]==buffer2[10]&&buffer1[15]==buffer2[15]){
				printf("\n====\n");
				PrintState(s1);
				PrintState(sb1);
				PrintState(s2);
				PrintState(sb2);
				printf("\n---\n");
				printf("\n%lld\n",count);
				return;
			}

			if(b==0xFFFFFFFF)
				break;
		}*/
		if(a==0xFFFFFFFF)
			break;
	}
	printf("%lu",sizeof(unsigned int));
}

void attack(){
	unsigned int x;
	long count=0;
	int i,j;
	for(std::map<unsigned int,vector<unsigned int> >::iterator iter = desiredPairsMap.begin(); iter != desiredPairsMap.end(); ++iter)
	{
		vector<unsigned int> values = iter->second;
		for (i = 0; i < values.size(); ++i)
		{
			for (j = 0; j < values.size(); ++j){
				x=values[i]^values[j];
				std::map<int,vector<unsigned int> >::iterator iter2	= hashTable.find(x);
				if(iter2!=hashTable.end()){
					vector<unsigned int> inputs = iter2->second;
					for(std::vector<unsigned int>::iterator iter3 = inputs.begin(); iter3 != inputs.end(); ++iter3){
						unsigned int invalidkey = values[i]^(*iter3);
						if(!nthbit(invalidkey)){
							count++;
						}
						setnthbit(invalidkey);
						invalidkey = values[j]^(*iter3);
						if(!nthbit(invalidkey)){
							count++;
						}
						setnthbit(invalidkey);
					}
				}
			}
		}
	}
	printf("%ld",count);
}



int main(int argc, char *argv[])
{
    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	//DesiredPairs(key);
    //PreComputation();
	//unsigned int a=0;
	//setnthbit(&a,7);
	//printf("\n%u\n",nthbit(a,8) );
    memset(keyInvalid,0, sizeof(unsigned int)*1<<27);
    return 0;
}
