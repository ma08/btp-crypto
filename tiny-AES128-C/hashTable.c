//#include <boost/serialization/vector.hpp>
//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/serialization/map.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>

#include "aes.h"
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
//#include<bits/stdc++.h>


typedef uint8_t state_t[4][4];
//using namespace std;
//using namespace boost;
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


//map<int,std::vector<unsigned int> >hashTable;
//typedef struct node{
typedef struct __attribute__((__packed__)) node {
    unsigned int p;
    struct node* next;
}Node;


void write(Node** hashLinkedList){
    Node *temp,*temp2;
    unsigned long i,j;
    unsigned long count=0;
    unsigned int *temparr=malloc(sizeof(int)*(long)1<<32);
    FILE * pFile;
    pFile = fopen ("hashTable2.bin", "wb");
    for(i=0;i<(long)1<<32;i++){
        j=0;
        temp=hashLinkedList[i];
        while(temp!=NULL){
            j++;
            temparr[j]=temp->p;
            temp=temp->next;
        }
        count+=j;
        temparr[0]=j;
        fwrite (temparr , sizeof(int), j+1, pFile);
    }
    printf("woooiii %lu ",count);
    fclose(pFile);
    free(temparr);
}


void PreComputation(){
    int a1, a2, b, c, d;
    unsigned int hashKey=0;
    int i;
    uint8_t *c1;
    uint8_t *c2;
    c1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    c2 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    unsigned long count=0;
    unsigned long count2=0;
    Node** hashLinkedList;
    Node* temp,temp2 ;
    hashLinkedList = (Node**)calloc((long)1<<32,sizeof(Node*));
    Node* X= (Node*)malloc(sizeof(Node)*((long)1<<33));
    //Node* X= (Node*)malloc(((long)1<<32),sizeof(Node));
    for (a1 = (1<<8)-1; a1 >= 0; a1--)
        for (a2 = a1-1; a2 >= 0; a2--)
            for (b = 0; b < 1<<8; ++b)
                for (c = 0; c < 1<<8; ++c)
                    for (d = 0; d < 1<<8; ++d){
                        count2++;
                        if(count2<=((long)1)<<33)
                           continue;
                        count++;
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
                        InvCipherSinglePreComp(c1);
                        InvCipherSinglePreComp(c2);
                        hashKey=0;
                        hashKey=hashKey|(((*s1)[0][0]^(*s2)[0][0])<<24);
                        hashKey=hashKey|(((*s1)[1][1]^(*s2)[1][1])<<16);
                        hashKey=hashKey|(((*s1)[2][2]^(*s2)[2][2])<<8);
                        hashKey=hashKey|(((*s1)[3][3]^(*s2)[3][3]));
                        temp = &(X[count-1]);
                        temp->p=bufToInt(c1);
                        //printf("\n%lu %lu",hashKey,temp->p);
                        temp->next = hashLinkedList[hashKey];
                        hashLinkedList[hashKey]= temp;
                        if(count==(((long)1)<<33)){
                            printf("%lld woooooo\n",count);
                            fflush(stdout);
                            write(hashLinkedList);
                            return;
                        }
                        if(count%(1<<24)==0){
                            printf("%lld\n",count);
                            fflush(stdout);
                        }
                    }
    write(hashLinkedList);
}

/*
void run(){
    filesystem::path myFile = filesystem::current_path() / "myfile1.dat";
    if (filesystem::exists(myFile))
    {
        filesystem::ifstream ifs(myFile);
        archive::text_iarchive ta(ifs);

        ta >> hashTable; // foo is empty until now, it's fed by myFile

        std::cout << "Read " << hashTable.size() << " entries from " << myFile << "\n";
    }else{
        PreComputation();
        filesystem::ofstream ofs(myFile);
        archive::text_oarchive ta(ofs);

        ta << hashTable; // foo is empty until now, it's fed by myFile
        std::cout << "Wrote " << hashTable.size() << " random entries to " << myFile << "\n";
    }
}
*/

int main(int argc, char *argv[])
{
    //run();
    PreComputation();
    return 0;
}
