#include "aes.h"
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>


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

void PrintBuf(uint8_t *buf){
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

unsigned int bufToInt(uint8_t *buf){
    unsigned int a;
    a=0;
    a=a|(buf[0]<<24);
    a=a|(buf[5]<<16);
    a=a|(buf[10]<<8);
    a=a|(buf[15]);
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
int nthbit(unsigned int n, unsigned int* keyInvalid){
    unsigned int ind1 = n/32;
    unsigned int indbit = n%32;
    return keyInvalid[ind1]&(1<<(32-indbit-1));
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
    for(i=0;i<(long)1<<32;i++){
        fread(&count1,sizeof(unsigned int),1,hfile);
        sum+=count1;
        countarr[i]=sum;
        fread(cur,sizeof(unsigned int),count1,hfile);
        cur+=count1;
    }
    close(hfile);
}

void generateDesiredPairs(uint8_t* key, Node** desiredPairs, Node* X, int diagonal){
    uint8_t *p1,*out;
    p1 = (uint8_t *)calloc(16,sizeof(uint8_t));
    out = (uint8_t *)calloc(16,sizeof(uint8_t));
    state_t* s1 = (state_t*)p1;
    int  a, b, c, d;
    unsigned long count=0;
    unsigned int hashkey;
    Node* temp,*temp2 ;
    for (a = 0; a < 1<<8; a++)
        for (b = 0; b < 1<<8; ++b)
            for (c = 0; c < 1<<8; ++c)
                for (d = 0; d < 1<<8; ++d){
                    count++;
                    (*s1)[0][0]=a;
                    (*s1)[1][1]=b;
                    (*s1)[2][2]=c;
                    (*s1)[3][3]=d;
                    AES128_ECB_encrypt(p1, key, 5, out);
                    // hashkey=0;
                    hashkey = bufToInt_Cipher(out,diagonal);
                    // printf("%d\n",hashkey );
                    // cout<<hashkey<<endl;
                    // temp = (Node*)malloc(sizeof(Node));
                    temp = &(X[count-1]);
                    temp->plainText = bufToInt(p1);
                    // temp->plainText = 0;
                    temp->next = desiredPairs[hashkey];
                    desiredPairs[hashkey]= temp;

                    // hashTable[hashkey].push_back(bufToInt(p1));
                    if(count%(1<<28)==0){
                        printf("\n%u",count);
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
                        printf("\n %lu %lu %d\n",pairs,invalid_count,nthbit(0,keyInvalid));
                        return invalid_count;
                    }
                }
            }
        }
        if(hkey%(1<<28)==0){
            printf("\n %u %lu",hkey,pairs);
            fflush(stdout);
        }
    }
    printf("\n %lu %lu %d\n",pairs,invalid_count,nthbit(0,keyInvalid));
    return invalid_count;
}


int main(int argc, char *argv[])
{
    //uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned long invalid_count=0;
    int diagonal=0;
    Node** desiredPairs;
    desiredPairs = (Node**)calloc((long)1<<32,sizeof(Node*));
    Node* X= (Node*)malloc(sizeof(Node)*((long)1<<32));
    unsigned long* countarr_orig= (unsigned long*)malloc(sizeof(long)*(((long)1<<32)+1));
    unsigned long* countarr = countarr_orig+1;
    unsigned int *hasharr=malloc(sizeof(int)*((long)1<<34));
    unsigned int *keyInvalid=calloc((((long)1<<32)/32),sizeof(int));
    countarr[-1]=0;
    read_hashtable(countarr,hasharr);
    printf("\n read hash table");
    fflush(stdout);
    for(diagonal=0;diagonal<4;diagonal++){
        generateDesiredPairs(key,desiredPairs,X,diagonal);
        printf("\n desired pairs generation completed %d",diagonal);
        fflush(stdout);
        invalid_count = eliminateKeys(desiredPairs,countarr,hasharr,keyInvalid,invalid_count);
        printf("\n elimination %d diagonal completed %lu %d",diagonal,invalid_count,nthbit(0,keyInvalid));
        fflush(stdout);
        memset(desiredPairs,0,sizeof(Node *)*((long)1<<32));
    }
    return 0;
}


