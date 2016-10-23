//#include "aes.h"
#include "../aes_imp/aes_intrinsic.h"
#include "aes.h"
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include <time.h>
#include <stdlib.h>

#define FLUSH fflush(stdout);

#define NUM_PLAINTEXTS 25

ALIGN16 const uint8_t key_global[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0x2e, 0x7a, 0x15, 0x12, 0x25, 0xaa, 0xce, 0xa2, 0xa7, 0xf3, 0x11, 0x82, 0x05, 0xc7, 0x3f, 0x2f
    };

typedef uint8_t state_t[4][4];

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


//typedef struct node{
typedef struct __attribute__((__packed__)) node {
    unsigned int plainText;
    struct node* next;
}Node;

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

void generateDesiredPairs(const uint8_t* key_256, Node** desiredPairs, Node* X, int partial_diagonal){
    ALIGN16 uint8_t p1[16],p1_copy[16],out[16];
    state_t* s1 = (state_t*)p1;
    state_t* s1_copy = (state_t*)p1_copy;
    int  a, b, c, d;
    unsigned long count=0;
    uint8_t hashkey;
    Node* temp,*temp2 ;
    for (a = 0; a < 1<<8; a++)
        for (b = 0; b < 1<<8; ++b)
            for (c = 0; c < 1<<8; ++c)
                for (d = 0; d < 1<<8; ++d){

                    memset(p1,0,sizeof(uint8_t)*16);
                    memset(p1_copy,0,sizeof(uint8_t)*16);
                    /*memset(out,0,sizeof(uint8_t)*16);*/


                    count++;
                    if(count > 1<<NUM_PLAINTEXTS){
                        printf("\n%lu plaintext count completed",count);
                        FLUSH;
                        return ;

                    }
                    (*s1)[0][partial_diagonal]=a;
                    (*s1)[1][(partial_diagonal+1)%4]=b;
                    (*s1)[2][(partial_diagonal+2)%4]=c;
                    (*s1)[3][(partial_diagonal+3)%4]=d;

                    (*s1_copy)[0][partial_diagonal]=a;
                    (*s1_copy)[1][(partial_diagonal+1)%4]=b;
                    (*s1_copy)[2][(partial_diagonal+2)%4]=c;
                    (*s1_copy)[3][(partial_diagonal+3)%4]=d;

                    InvCipherSingleRound(p1,key_256);
                    AES256_ECB_encrypt(p1, key_256, 5, out);
                    /*PrintBuf(out);*/
                    hashkey = out[0];
                    /*printf("\n out[0] is %x",hashkey);*/
                    temp = &(X[count-1]);
                    temp->plainText = bufToInt_Plain(p1_copy,partial_diagonal);
                    temp->next = desiredPairs[hashkey];
                    desiredPairs[hashkey]= temp;
/*
                    if((a==0x1 && b==0x7e && c== 0x78 && d==0x95)||(a==0x00 && b==0x20 && c==0x1c && d==0xc3)){
                        printf("\n%x %x %x %x",a,b,c,d);
                        PrintBuf(out);
                        printf("\n out[0] is %x",hashkey);

                    }*/


                    //remove this
                    /*a=0x1;
                    b=0x7e;
                    c=0x78;
                    d=0x95;

                    memset(p1,0,sizeof(uint8_t)*16);
                    memset(p1_copy,0,sizeof(uint8_t)*16);
                    memset(out,0,sizeof(uint8_t)*16);


                    count++;
                    if(count > 1<<NUM_PLAINTEXTS){
                        printf("\n%lu plaintext count completed",count);
                        FLUSH;
                        return ;

                    }
                    (*s1)[0][partial_diagonal]=a;
                    (*s1)[1][(partial_diagonal+1)%4]=b;
                    (*s1)[2][(partial_diagonal+2)%4]=c;
                    (*s1)[3][(partial_diagonal+3)%4]=d;

                    (*s1_copy)[0][partial_diagonal]=a;
                    (*s1_copy)[1][(partial_diagonal+1)%4]=b;
                    (*s1_copy)[2][(partial_diagonal+2)%4]=c;
                    (*s1_copy)[3][(partial_diagonal+3)%4]=d;

                    InvCipherSingleRound(p1,key_256);
                    AES256_ECB_encrypt(p1, key_256, 5, out);
                    PrintBuf(out);
                    hashkey = out[0];
                    printf("\n out[0] is %x",hashkey);
                    temp = &(X[count-1]);
                    temp->plainText = bufToInt_Plain(p1_copy,partial_diagonal);
                    temp->next = desiredPairs[hashkey];
                    desiredPairs[hashkey]= temp;*/


                }
}

unsigned int eliminateKeys(Node** desiredPairs,unsigned long* countarr,unsigned int* hasharr,unsigned int* keyInvalid,unsigned int start_count){
    Node* temp,*temp2 ;
    unsigned long invalid_count=start_count;
    unsigned long hkey,i;
    unsigned int pkey,value;
    unsigned long pairs=0;

    unsigned int actual_key=bufToInt_Plain(key_global+16,0);

    for(hkey=0;hkey<(long)1<<8;hkey++){
        unsigned long pair_now=0;
        for(temp=desiredPairs[hkey];temp!=NULL;temp=temp->next){
            //remove this
            /*printf("\n here ");*/
            for(temp2=temp->next;temp2!=NULL;temp2=temp2->next){
                //remove this
                /*printf("\n there ");*/
                pairs++;
                pair_now++;
                pkey=temp->plainText^temp2->plainText;
                /*printf("\n%u %lu",pkey,countarr[pkey]-countarr[pkey-1]);*/
                for(i=countarr[pkey-1];i<countarr[pkey];i++){
                    value=hasharr[i]^temp->plainText;

                    if(value==actual_key){
                        printf("\n---culprits--hkey %lx---\n",hkey);
                        printf("\n%x",temp->plainText);
                        printf("\n%x\n",temp2->plainText);
                        return invalid_count;

                    }
                    if(!nthbit(value,keyInvalid)){
                        setnthbit(value,keyInvalid);
                        invalid_count++;
                    }
                    value=hasharr[i]^temp2->plainText;

                    if(value==actual_key){
                        printf("\n---culprits--hkey %lx---\n",hkey);
                        printf("\n%x",temp->plainText);
                        printf("\n%x\n",temp2->plainText);
                        return invalid_count;
                    }


                    if(!nthbit(value,keyInvalid)){
                        invalid_count++;
                        setnthbit(value,keyInvalid);
                    }
                    if(invalid_count>=((long)1<<32)-300){
                        printf("\n %lu %lu\n",pairs,invalid_count);
                        return invalid_count;
                    }
                }
            }
        }
        printf("\n %lu %lu %lu %lu %d\n",hkey,pairs,pair_now,invalid_count,nthbit(actual_key,keyInvalid));
        fflush(stdout);
    }
    printf("\n %lu %lu\n",pairs,invalid_count);
    return invalid_count;
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

void attack(int partial_diagonal,const uint8_t* key){
    /*uint8_t key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};*/
    unsigned long invalid_count=0;
    Node** desiredPairs;
    //1 byte
    desiredPairs = (Node**)calloc((long)1<<8,sizeof(Node*));

    //2^25 plaintexts
    Node* X= (Node*)malloc(sizeof(Node)*((long)1<<NUM_PLAINTEXTS));
    unsigned long* countarr_orig= (unsigned long*)malloc(sizeof(long)*(((long)1<<32)+1));
    unsigned long* countarr = countarr_orig+1;
    unsigned int *hasharr=(unsigned int *)malloc(sizeof(int)*((long)1<<34));
    unsigned int *keyInvalid=(unsigned int *)calloc((((long)1<<32)/32),sizeof(int));
    countarr[-1]=0;
    read_hashtable(countarr,hasharr);
    printf("\n read hash table");
    fflush(stdout);
    
    printf("------------ PARTIAL DIAGONAL ------------ %d",partial_diagonal);
    fflush(stdout);
    generateDesiredPairs(key,desiredPairs,X,partial_diagonal);
    printf("\n desired pairs generation completed");
    FLUSH;
    invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);


    printf("\n elimination completed %lu %d",invalid_count,nthbit(bufToInt_Plain(key+16,partial_diagonal),keyInvalid));

    /*for(diagonal=0;diagonal<4;diagonal++){
        generateDesiredPairs(key,desiredPairs,X,diagonal,partial_diagonal);
        fflush(stdout);
        invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);
        printf("\n elimination %d diagonal completed %lu %d",diagonal,invalid_count,nthbit(bufToInt_Plain(key,partial_diagonal),keyInvalid));
        fflush(stdout);
        memset(desiredPairs,0,sizeof(Node *)*((long)1<<32));
    }*/

    write_valid_keys(invalid_count,keyInvalid,partial_diagonal);
    return;
}

//Checks if two states are equal
int check_equal(uint8_t* b1,uint8_t* b2){
    int i;
    for(i=0;i<16;i++)
        if(b1[i]!=b2[i])
            return 0;
    return 1;
}


//Takes a plaintext and ciphertex pair and gets possible partial keys from the saved files
//and tests the keys for possible match with the pair.
void bruteforce_partial_given(uint8_t* plaintext, uint8_t* ciphertext){
    uint8_t key[16];
    uint8_t out[16];
    FILE *kfiles[4];
    unsigned int *partial_keys[4];
    unsigned int keys_count[4];
    unsigned int key_partials[4];
    unsigned long i,j,k,l;
    char filename[] = "partialkey0.bin";
    for(i=0;i<4;i++){
        filename[10]=i-0+'0';
        kfiles[i] = fopen (filename, "r");
        printf("\n%s",filename);
    }
    for(i=0;i<4;i++){
        fread(&keys_count[i],sizeof(unsigned int),1,kfiles[i]);
        printf("\n %lu %u %d",i,keys_count[i],4+keys_count[i]*4);
        fflush(stdout);
        partial_keys[i]=(unsigned int *)(malloc(sizeof(int)*keys_count[i]));
        
        //printf("\n %u",sizeof(partial_keys[i]));
        //fflush(stdout);
        unsigned int x = fread(partial_keys[i],sizeof(unsigned int),keys_count[i],kfiles[i]);

        //remove this
        break;
        /*printf("\n%x %x %x %x\n",partial_keys[0][0]>>24, partial_keys[0][0]>>16,partial_keys[0][0]>>8, partial_keys[0][0]);*/
        /*return;*/
        //printf(" %u",x);
    }
    for(i=0;i<4;i++){
        printf("\n partial keys diagonal %lu",i);
        for(j=0;j<keys_count[i];j++){
            printf("\n%x ",partial_keys[i][j]);
        }
        return;
    }
    fflush(stdout);
    return;
    unsigned long count=0;
    for(i=0;i<keys_count[0];i++){
        key_partials[0]=partial_keys[0][i];
        for(j=0;j<keys_count[1];j++){
            key_partials[1]=partial_keys[1][j];
            for(k=0;k<keys_count[2];k++){
                key_partials[2]=partial_keys[2][k];
                for(l=0;l<keys_count[3];l++){
                    key_partials[3]=partial_keys[3][l];
                    count++;
                    //printf("\n%lu",count);
                    //fflush(stdout);
                    build_key(key_partials,key);
                    AES128_ECB_encrypt(plaintext, key, 5, out);
                    if(check_equal(out,ciphertext)){
                        printf("\npossible Key found");
                        PrintBuf(key);
                        return;
                    }
                    if(count%(1<<28)==0){
                        printf("\n%lu",count);
                        fflush(stdout);
                    }
                }
            }
        }
    }
    printf("\nno key found :(");
    return;
}

//GIven a key, generate a plaintext and ciphertext pair and run bruteforce_partial_given
void bruteforce_partial(const uint8_t* key){
    uint8_t *p1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    uint8_t *out = (uint8_t *)malloc(sizeof(uint8_t)*16);
    int i=0; 
    for(i=0;i<16;i++)
        p1[i]=0;
    AES256_ECB_encrypt(p1, key, 5, out);
    bruteforce_partial_given(p1,out);
}



void check_culprits(unsigned int p1,unsigned int p2){
    ALIGN16 uint8_t buf1[16],buf2[16],buf1_copy[16],buf2_copy[16],out1[16],out2[16],out3[16],out4[16];

    /*state_t* s1 = (state_t*)buf1;*/
    /*state_t* s2 = (state_t*)buf2;*/
    intToBuf_Plain(buf1,0,p1);
    intToBuf_Plain(buf2,0,p2);

    printf("\n%x ",bufToInt_Plain(buf1,0));
    printf("\n%x ",bufToInt_Plain(buf2,0));

    /*intToBuf_Plain(buf1_copy,0,p1);*/
    /*intToBuf_Plain(buf2_copy,0,p2);*/

    PrintBuf(buf1);
    PrintBuf(buf2);

    InvCipherSingleRound(buf1, key_global);
    InvCipherSingleRound(buf2, key_global);

    /*InvCipherSingleRound(buf1_copy, key_global);*/
    /*InvCipherSingleRound(buf2_copy, key_global);*/

    printf("\n-------PLAINTEXTS--------\n");
    PrintBuf(buf1);
    PrintBuf(buf2);

    printf("\n-------END OF 2nd Round--------\n");
    AES256_ECB_encrypt_inclastmix(buf1,key_global,2,out1);
    AES256_ECB_encrypt_inclastmix(buf2,key_global,2,out2);
    PrintBuf(out1);
    PrintBuf(out2);
    fflush(stdout);
    printf("\n-------END OF 5th Round--------\n");
    AES256_ECB_encrypt(buf1,key_global,5,out1);
    AES256_ECB_encrypt(buf2,key_global,5,out2);
    PrintBuf(out1);
    PrintBuf(out2);

    printf("\n%x",out1[0]);
    printf("\n%x",out2[0]);

    unsigned long* countarr_orig= (unsigned long*)malloc(sizeof(long)*(((long)1<<32)+1));
    unsigned long* countarr = countarr_orig+1;
    unsigned int *hasharr=(unsigned int *)malloc(sizeof(int)*((long)1<<34));
    countarr[-1]=0;
    read_hashtable(countarr,hasharr);

    printf("\n---Eliminated keys--\n");
    uint32_t hkey=p1^p2,value;
    for(int i=countarr[hkey-1];i<countarr[hkey];i++){
        value=hasharr[i]^p1;
        printf("\n%x",value);
        value=hasharr[i]^p2;
        printf("\n%x",value);
    }
}








int main(int argc, char *argv[])
{
    int partial_diagonal=0;
    if(argc>1){
        partial_diagonal=argv[1][0]-'0';
    }

    /*//256 key
    ALIGN16 const uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0x2e, 0x7a, 0x15, 0x12, 0x25, 0xaa, 0xce, 0xa2, 0xa7, 0xf3, 0x11, 0x82, 0x05, 0xc7, 0x3f, 0x2f
    };*/

    const uint8_t*key = key_global;
    printf("\nPartial key %x \n",bufToInt_Plain(key+16,0));
    /*check_culprits(0x17e7895,0x201cc3);*/

    /*uint8_t a;
    uint32_t b= 0xbca3aefb;
    a=b;
    printf("\n%x",a);*/

    /*attack(partial_diagonal,key);*/
    bruteforce_partial(key);
    //test_write();
    return 0;
}


