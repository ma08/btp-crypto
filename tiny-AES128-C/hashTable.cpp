#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "aes.h"
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<bits/stdc++.h>


typedef uint8_t state_t[4][4];
using namespace std;
using namespace boost;
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
map<int,std::vector<unsigned int> >hashTable;
void PreComputation(){
    int a1, a2, b, c, d;
    int hashKey=0;
    int i;
    uint8_t *c1;
    uint8_t *c2;
    c1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    c2 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    long long count=0;
    for (a1 = (1<<8)-1; a1 >= 0; a1--)
        for (a2 = a1-1; a2 >= 0; a2--)
            for (b = 0; b < 1<<8; ++b)
                for (c = 0; c < 1<<8; ++c)
                    for (d = 0; d < 1<<8; ++d){
                        state_t* s1 = (state_t*)c1;
                        state_t* s2 = (state_t*)c2;
                        (*s1)[0][0]=a1;
                        (*s2)[0][0]=a2;
                        (*s2)[0][1]=(*s1)[0][1]=b;
                        (*s2)[0][2]=(*s1)[0][2]=c;
                        (*s2)[0][3]=(*s1)[0][3]=d;
                        for (i = 4; i < 16; ++i)
                        {
                            c1[i]=c2[i]=0;
                        }
                        //printf("-------------------\n");
                        //PrintState(s1);
                        //PrintState(s2);
                        InvCipherSinglePreComp(c1);
                        InvCipherSinglePreComp(c2);
                        //PrintState(s1);
                        //PrintState(s2);
                        //printf("\nHash value = %x %x %x %x\n",(*s1)[0][0]^(*s2)[0][0],(*s1)[1][1]^(*s2)[1][1],(*s1)[2][2]^(*s2)[2][2],(*s1)[3][3]^(*s2)[3][3]);
                        hashKey=0;
                        hashKey=hashKey|(((*s1)[0][0]^(*s2)[0][0])<<24);
                        hashKey=hashKey|(((*s1)[1][1]^(*s2)[1][1])<<16);
                        hashKey=hashKey|(((*s1)[2][2]^(*s2)[2][2])<<8);
                        hashKey=hashKey|(((*s1)[3][3]^(*s2)[3][3]));
                        //printf("Storing value %x\n",hashKey );

                        if (hashTable.find(hashKey) == hashTable.end())
                        {
                            vector<unsigned int> temp;
                            hashTable[hashKey] = temp;
                        }
                        hashTable[hashKey].push_back(bufToInt(c1));
                        //hashTable[hashKey].push_back(bufToInt(c2));
                        count++;
                        //PrintState((state_t*)c1);
                        //printf("=====================\n");
                        if(count==(((long)1)<<32)){
                            printf("%lld woooooo\n",count);
			    fflush(stdout);
                            return;
                        }
                        if(count%(1<<20)==0 && count/(1<<20) >0){
                            printf("%lld\n",count);
			    fflush(stdout);
			}
                        /*PrintState(s1);*/
                        /*PrintState(s2);*/

                        /*return;*/
                    }
}

void run(){
	filesystem::path myFile = filesystem::current_path() / "myfile.dat";
	if (filesystem::exists(myFile))
	{
		filesystem::ifstream ifs(myFile/*.native()*/);
		archive::text_iarchive ta(ifs);

		ta >> hashTable; // foo is empty until now, it's fed by myFile

		std::cout << "Read " << hashTable.size() << " entries from " << myFile << "\n";
	}else{
		PreComputation();
		filesystem::ofstream ofs(myFile/*.native()*/);
		archive::text_oarchive ta(ofs);

		ta << hashTable; // foo is empty until now, it's fed by myFile
		std::cout << "Wrote " << hashTable.size() << " random entries to " << myFile << "\n";
	}
}

int main(int argc, char *argv[])
{
    run();
    return 0;
}
