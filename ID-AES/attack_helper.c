#include "attack_helper.h"
#include<stdint.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define FLUSH fflush(stdout);


/*typedef uint8_t state_t[4][4];*/

void PrintState(state_t* s){
    printf("\n");
    int i,j;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
            printf("%x ",(*s)[j][i]);
        printf("\n");
    }
    printf("\n");
}

void PrintBuf(uint8_t *buf){
    printf("\n");
    state_t* s = (state_t*)buf;
    int i,j;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
            printf("%x ",(*s)[j][i]);
        printf("\n");
    }
    printf("\n");
}
/*
unsigned int bufToInt(uint8_t *buf){
    unsigned int a;
    a=0;
    a=a|(buf[0]<<24);
    a=a|(buf[5]<<16);
    a=a|(buf[10]<<8);
    a=a|(buf[15]);
    return a;
}*/

unsigned int bufToInt_Plain(const uint8_t *buf,int partial_diagonal){
    unsigned int a;
    state_t* s = (state_t*)buf;
    a=0;
    a=a|((*s)[0][partial_diagonal]<<24);
    a=a|((*s)[1][(partial_diagonal+1)%4]<<16);
    a=a|((*s)[2][(partial_diagonal+2)%4]<<8);
    a=a|((*s)[3][(partial_diagonal+3)%4]);
    return a;
}


unsigned int bufToInt_Cipher(uint8_t *buf,int diagonal){
    unsigned int a;
    a=0;
    state_t* s = (state_t*)buf;
    a=a|(((*s)[0][diagonal])<<24);
    a=a|(((*s)[1][(3+diagonal)%4])<<16);
    a=a|(((*s)[2][(2+diagonal)%4])<<8);
    a=a|(((*s)[3][(1+diagonal)%4]));
    /*a=a|(buf[0]<<24);
      a=a|(buf[5]<<16);
      a=a|(buf[10]<<8);
      a=a|(buf[15]);*/
    return a;
}

//Built complete key from partial keys.
void build_key(unsigned int* partial_keys,uint8_t* key){
    int i,j;
    state_t* s = (state_t*)key;
    for(i=0;i<4;i++){
        (*s)[0][i]=partial_keys[i]>>24;
        (*s)[1][(i+1)%4]=partial_keys[i]>>16;
        (*s)[2][(i+2)%4]=partial_keys[i]>>8;
        (*s)[3][(i+3)%4]=partial_keys[i];
    }
    return;
}



void intToBuf_Plain(uint8_t *buf,int partial_diagonal,const unsigned int a){
    memset(buf,0,sizeof(uint8_t)*16);

    state_t* s1 = (state_t*)buf;

    /*for (i = 0; i < 4; ++i){
        y = 0xFF<<(i*8);
        buf[i*5] = (a&y)>>(i*8);
    }*/
    (*s1)[0][partial_diagonal]=a>>24;
    (*s1)[1][(partial_diagonal+1)%4]=a>>16;
    (*s1)[2][(partial_diagonal+2)%4]=a>>8;
    (*s1)[3][(partial_diagonal+3)%4]=a;
}

//0 indexed
int nthbit(unsigned int n, unsigned int* keyInvalid){
    unsigned int ind1 = n/32;
    unsigned int indbit = n%32;
    return (keyInvalid[ind1]&(1<<(32-indbit-1)))>>(32-indbit-1);
}

void unsetnthbit(unsigned int n, unsigned int* keyInvalid){
    unsigned int ind1 = n/32;
    unsigned int indbit = n%32;
    unsigned int x = ((long)1<<32)-1;
    unsigned int y = 1<<31;
    //printf("\n%u\n",x);
    y = y>>(indbit);
    x=x-y;
    //printf("\n%u\n",x);
    keyInvalid[ind1]=keyInvalid[ind1]&x;
}
void setnthbit(unsigned int n, unsigned int* keyInvalid){
    unsigned int ind1 = n/32;
    unsigned int indbit = n%32;
    unsigned int x = 1<<31;
    //printf("\n%u\n",x);
    x = x>>(indbit);
    //printf("\n%u\n",x);
    keyInvalid[ind1]=keyInvalid[ind1]|x;
}

// map<unsigned int,std::vector<unsigned int> >hashTable;


void read_hashtable(unsigned long* countarr,unsigned int* hasharr){
    FILE *hfile;
    hfile = fopen ("hashtable.bin", "r");
    unsigned long i,j,sum=0;
    unsigned int count1,count2;
    unsigned int *cur=hasharr;
    unsigned long numzeroes=0;
    for(i=0;i<(long)1<<32;i++){
        fread(&count1,sizeof(unsigned int),1,hfile);
        if(count1==0)
            numzeroes++;
        sum+=count1;
        countarr[i]=sum;
        fread(cur,sizeof(unsigned int),count1,hfile);
        cur+=count1;
    }
    printf("\n hash table entries %lu. number of zeroes %lu",sum,numzeroes);
    FLUSH;
    fclose(hfile);
}

void write_valid_keys(unsigned long invalid_count, unsigned int *keyInvalid,int partial_diagonal){
    //fuck me
    unsigned long valid_count =((long)1<<32) - invalid_count;
    printf("\ninvalid_count %lu\n",invalid_count);
    printf("\nvalid_count %lu\n",valid_count);
    printf("\ntotal %lu\n",invalid_count+valid_count);
    fflush(stdout);
    unsigned int* validkeys_orig = (unsigned int*)(malloc(sizeof(unsigned int)*(valid_count+1)));
    unsigned int* validkeys = validkeys_orig+1;
    char filename[] = "partialkey0.bin";
    filename[10]=partial_diagonal-0+'0';
    FILE * pFile;
    pFile = fopen (filename, "wb");
    unsigned long count=0;
    unsigned long i; 
    for(i=0;i<((long)1<<32);i++){
        if(!nthbit(i,keyInvalid)){
            validkeys[count]=i;
            count++;
        }
    }
    printf("COUNT  %lu %lu",valid_count,count);
    fflush(stdout);
    validkeys_orig[0]=count;
    if(count!=valid_count){
        printf("COUNT MISMATCHHHHHHHHHHHHHHHHHHhhhhhhhhhhhhhhh %lu %lu",valid_count,count);
        fflush(stdout);
        return;
    }
    fwrite (validkeys_orig , sizeof(int), count+1, pFile);
    fclose(pFile);
    //free(validkeys);
}

int check_equal(uint8_t* b1,uint8_t* b2){
    int i;
    for(i=0;i<16;i++)
        if(b1[i]!=b2[i])
            return 0;
    return 1;
}

void test_write(){
    srand(time(NULL));
    unsigned int *keyInvalid=(unsigned int *)calloc((((long)1<<32)/32),sizeof(int));
    memset(keyInvalid,(1<<8)-1,sizeof(unsigned int)*((((long)1<<32)/32)));
    unsigned long i,count=(long)1<<32;
    for(i=0;i<(long)1<<20;i++){
        if((rand()%1000)<100){
            unsetnthbit(i,keyInvalid);
            count--;
        }
    }
    printf("\ncount %lu",count);
    write_valid_keys(count,keyInvalid,0);
    fflush(stdout);
}
