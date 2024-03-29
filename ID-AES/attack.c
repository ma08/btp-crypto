//#include "aes.h"
#include "../aes_imp/aes_intrinsic.h"
#include "attack_helper.h"
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include <time.h>
#include <stdlib.h>

ALIGN16 const uint8_t key_global[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};


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


void generateDesiredPairs(const uint8_t* key, Node** desiredPairs, Node* X, int diagonal, int partial_diagonal){
    
    ALIGN16 uint8_t p1[16],out[16];
    state_t* s1 = (state_t*)p1;

    int  a, b, c, d;
    unsigned long count=0;
    unsigned int hashkey;
    Node* temp,*temp2 ;
    for (a = 0; a < 1<<8; a++)
        for (b = 0; b < 1<<8; ++b)
            for (c = 0; c < 1<<8; ++c)
                for (d = 0; d < 1<<8; ++d){

                    memset(p1,0,sizeof(uint8_t)*16);

                    count++;
                    (*s1)[0][partial_diagonal]=a;
                    (*s1)[1][(partial_diagonal+1)%4]=b;
                    (*s1)[2][(partial_diagonal+2)%4]=c;
                    (*s1)[3][(partial_diagonal+3)%4]=d;
                    AES128_ECB_encrypt(p1, key, 5, out);
                    // hashkey=0;
                    hashkey = bufToInt_Cipher(out,diagonal);
                    // printf("%d\n",hashkey );
                    // cout<<hashkey<<endl;
                    // temp = (Node*)malloc(sizeof(Node));
                    temp = &(X[count-1]);
                    temp->plainText = bufToInt_Plain(p1,partial_diagonal);
                    // temp->plainText = 0;
                    temp->next = desiredPairs[hashkey];
                    desiredPairs[hashkey]= temp;

                    // hashTable[hashkey].push_back(bufToInt(p1));
                    if(count%(1<<28)==0){
                        printf("\n%lu",count);
                        // cout<<count<<endl;
                        fflush(stdout);
                        //free(p1);
                        //free(out);
                        //return 0;
                    }
                }
}

unsigned int eliminateKeys(Node** desiredPairs,unsigned long* countarr,unsigned int* hasharr,unsigned int* keyInvalid,unsigned int start_count){
    Node* temp,*temp2 ;
    unsigned int invalid_count=start_count;
    unsigned long hkey,i;
    unsigned int pkey,value;
    unsigned long pairs=0;
    for(hkey=0;hkey<(long)1<<32;hkey++){
        for(temp=desiredPairs[hkey];temp!=NULL;temp=temp->next){
            for(temp2=temp->next;temp2!=NULL;temp2=temp2->next){
                pairs++;
                pkey=temp->plainText^temp2->plainText;
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
                    if(invalid_count==((long)1<<32)-1){
                        printf("\n %lu %u %d\n",pairs,invalid_count,nthbit(0,keyInvalid));
                        return invalid_count;
                    }
                }
            }
        }
        if(hkey%(1<<28)==0){
            printf("\n %lu %lu %u",hkey,pairs,invalid_count);
            fflush(stdout);
        }
    }
    printf("\n %lu %u %d\n",pairs,invalid_count,nthbit(0,keyInvalid));
    return invalid_count;
}



void attack(int partial_diagonal,const uint8_t* key){
    /*uint8_t key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};*/
    unsigned long invalid_count=0;
    int diagonal=0;
    Node** desiredPairs;
    desiredPairs = (Node**)calloc((long)1<<32,sizeof(Node*));
    Node* X= (Node*)malloc(sizeof(Node)*((long)1<<32));
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
    for(diagonal=0;diagonal<4;diagonal++){
        generateDesiredPairs(key,desiredPairs,X,diagonal,partial_diagonal);
        printf("\n desired pairs generation completed %d",diagonal);
        fflush(stdout);
        invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);
        printf("\n elimination %d diagonal completed %lu %d",diagonal,invalid_count,nthbit(bufToInt_Plain(key,partial_diagonal),keyInvalid));
        fflush(stdout);
        memset(desiredPairs,0,sizeof(Node *)*((long)1<<32));
    }
    write_valid_keys(invalid_count,keyInvalid,partial_diagonal);
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
        printf("\n %d %u %d",i,keys_count[i],4+keys_count[i]*4);
        fflush(stdout);
        partial_keys[i]=(unsigned int *)(malloc(sizeof(int)*keys_count[i]));
        //printf("\n %u",sizeof(partial_keys[i]));
        //fflush(stdout);
        unsigned int x = fread(partial_keys[i],sizeof(unsigned int),keys_count[i],kfiles[i]);
        //printf(" %u",x);
    }
    for(i=0;i<4;i++){
        for(j=0;j<keys_count[i];j++){
            printf("%u ",partial_keys[i][j]);
        }
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
void bruteforce_partial(uint8_t* key){
    uint8_t *p1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    uint8_t *out = (uint8_t *)malloc(sizeof(uint8_t)*16);
    int i=0; 
    for(i=0;i<16;i++)
        p1[i]=0;
    AES128_ECB_encrypt(p1, key, 5, out);
    bruteforce_partial_given(p1,out);
}


int main(int argc, char *argv[])
{
    int partial_diagonal=0;
    if(argc>1){
        partial_diagonal=argv[1][0]-'0';
    }

    const uint8_t*key = key_global;
    attack(partial_diagonal,key);
    //bruteforce_partial(key);
    //test_write();
    return 0;
}


