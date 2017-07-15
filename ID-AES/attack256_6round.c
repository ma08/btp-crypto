//#include "aes.h"
#include "../aes_imp/aes_intrinsic.h"
#include "aes.h"
#include "attack_helper.h"
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include <time.h>
#include <stdlib.h>



ALIGN16 const uint8_t key_global[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0x2e, 0x7a, 0x15, 0x12, 0x25, 0xaa, 0xce, 0xa2, 0xa7, 0xf3, 0x11, 0x82, 0x05, 0xc7, 0x3f, 0x2f
    };


void generateDesiredPairs(const uint8_t* key_256, Node** desiredPairs, Node* X, int diagonal, int partial_diagonal){
    
    ALIGN16 uint8_t p1[16],p1_copy[16],out[16];
    state_t* s1 = (state_t*)p1;
    state_t* s1_copy = (state_t*)p1_copy;

    int  a, b, c, d;
    unsigned long count=0;
    uint32_t hashkey;
    Node* temp,*temp2 ;
    for (a = 0; a < 1<<8; a++)
        for (b = 0; b < 1<<8; ++b)
            for (c = 0; c < 1<<8; ++c)
                for (d = 0; d < 1<<8; ++d){

                    memset(p1,0,sizeof(uint8_t)*16);
                    memset(p1_copy,0,sizeof(uint8_t)*16);

                    count++;
                    (*s1)[0][partial_diagonal]=a;
                    (*s1)[1][(partial_diagonal+1)%4]=b;
                    (*s1)[2][(partial_diagonal+2)%4]=c;
                    (*s1)[3][(partial_diagonal+3)%4]=d;

                    (*s1_copy)[0][partial_diagonal]=a;
                    (*s1_copy)[1][(partial_diagonal+1)%4]=b;
                    (*s1_copy)[2][(partial_diagonal+2)%4]=c;
                    (*s1_copy)[3][(partial_diagonal+3)%4]=d;

                    //using only 128 bits of the key here
                    InvCipherSingleRound(p1,key_256);

                    AES256_ECB_encrypt(p1, key_256, 6, out);
                    // hashkey=0;
                    hashkey = bufToInt_Cipher(out,diagonal);
                    // printf("%d\n",hashkey );
                    // cout<<hashkey<<endl;
                    // temp = (Node*)malloc(sizeof(Node));
                    temp = &(X[count-1]);
                    temp->plainText = bufToInt_Plain(p1_copy,partial_diagonal);
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

    unsigned int actual_key=bufToInt_Plain(key_global+16,0);


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
                        printf("\n %lu %u %d\n",pairs,invalid_count,nthbit(actual_key,keyInvalid));
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
    printf("\n %lu %u %d\n",pairs,invalid_count,nthbit(bufToInt_Plain(key_global+16,0),keyInvalid));
    return invalid_count;
}



void attack(int partial_diagonal,const uint8_t* key_256){
    /*uint8_t key_256[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};*/
    unsigned long invalid_count=0;
    int diagonal=0;
    Node** desiredPairs;
    desiredPairs = (Node**)calloc((long)1<<32,sizeof(Node*));
    Node* X= (Node*)malloc(sizeof(Node)*((long)1<<32));
    unsigned long* countarr_orig= (unsigned long*)malloc(sizeof(long)*(((long)1<<32)+1));
    unsigned long* countarr = countarr_orig+1;
    unsigned int *hasharr=(unsigned int *)malloc(sizeof(int)*((long)1<<34));
    unsigned int *keyInvalid=(unsigned int *)calloc((((long)1<<32)/32),sizeof(int));
    printf("\n%p %p %p %p %p\n",desiredPairs,X,countarr_orig,hasharr,keyInvalid);
    FLUSH;
    countarr[-1]=0;
    read_hashtable(countarr,hasharr);
    printf("\n read hash table");
    fflush(stdout);
    
    printf("------------ PARTIAL DIAGONAL ------------ %d",partial_diagonal);
    fflush(stdout);
    for(diagonal=0;diagonal<4;diagonal++){
        generateDesiredPairs(key_256,desiredPairs,X,diagonal,partial_diagonal);
        printf("\n desired pairs generation completed %d",diagonal);
        fflush(stdout);
        invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);
        //remove this
        if(invalid_count==0){
            return;
        }
        printf("\n elimination %d diagonal completed %lu %d",diagonal,invalid_count,nthbit(bufToInt_Plain(key_256+16,partial_diagonal),keyInvalid));
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



//FIX THIS
//GIven a key, generate a plaintext and ciphertext pair and run bruteforce_partial_given
void bruteforce_partial(const uint8_t* key){
    uint8_t *p1 = (uint8_t *)malloc(sizeof(uint8_t)*16);
    uint8_t *out = (uint8_t *)malloc(sizeof(uint8_t)*16);
    int i=0; 
    for(i=0;i<16;i++)
        p1[i]=0;
    AES128_ECB_encrypt(p1, key, 5, out);
    bruteforce_partial_given(p1,out);
}

/*void check_culprits(uint32_t p1, uint32_t p2){
    ALIGN16 uint8_t culprit1[16],culprit2[16], out1[16],out2[16];

    intToBuf_Plain(culprit1,0,p1);
    intToBuf_Plain(culprit2,0,p2);

    printf("\n start at 2nd round \n");
    PrintBuf(culprit1);
    PrintBuf(culprit2);

    printf("\n PLAINTEXTS \n");
    InvCipherSingleRound(culprit1,key_global);
    InvCipherSingleRound(culprit2,key_global);
    PrintBuf(culprit1);
    PrintBuf(culprit2);
    printf("\n End of 2nd round \n");
    AES256_ECB_encrypt_inclastmix(culprit1,key_global,2,out1);
    AES256_ECB_encrypt_inclastmix(culprit2,key_global,2,out2);
    PrintBuf(out1);
    PrintBuf(out2);
    printf("\n End of 6th round \n");
    AES256_ECB_encrypt(culprit1,key_global,6,out1);
    AES256_ECB_encrypt(culprit2,key_global,6,out2);
    PrintBuf(out1);
    PrintBuf(out2);


    printf("\nkeys %x %x\n",bufToInt_Cipher(out1,0),bufToInt_Cipher(out2,0));

}
*/
int main(int argc, char *argv[])
{
    int partial_diagonal=0;
    if(argc>1){
        partial_diagonal=argv[1][0]-'0';
    }

    const uint8_t*key = key_global;


    printf("\nPartial key %x \n",bufToInt_Plain(key+16,0));

    /*attack(partial_diagonal,key);*/
    bruteforce_partial(key);
    //test_write();
    /*uint32_t c1=0xcd0e1ae5,c2=0xfffffa2d;*/
    /*check_culprits(c1,c2);*/
    return 0;
}


