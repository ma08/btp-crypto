//#include "aes.h"
#include "../aes_imp/aes_intrinsic.h"
#include "attack_helper.h"
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

                    if(!nthbit(value,keyInvalid)){
                        setnthbit(value,keyInvalid);
                        invalid_count++;
                    }
                    value=hasharr[i]^temp2->plainText;
                    if(!nthbit(value,keyInvalid)){
                        invalid_count++;
                        setnthbit(value,keyInvalid);
                    }
                    if(invalid_count>=((long)1<<32)-100){
                        printf("\n %lu %lu\n",pairs,invalid_count);
                        FLUSH;
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
    FLUSH;

    /*for(diagonal=0;diagonal<4;diagonal++){
        generateDesiredPairs(key,desiredPairs,X,diagonal,partial_diagonal);
        fflush(stdout);
        invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);
        printf("\n elimination %d diagonal completed %lu %d",diagonal,invalid_count,nthbit(bufToInt_Plain(key,partial_diagonal),keyInvalid));
        fflush(stdout);
        memset(desiredPairs,0,sizeof(Node *)*((long)1<<32));
    }*/

    write_valid_keys(invalid_count,keyInvalid,partial_diagonal);
    FLUSH;
    return;
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

    //attack(partial_diagonal,key);
    //bruteforce_partial(key);
    //test_write();
    return 0;
}


