#include<stdint.h>

#define FLUSH fflush(stdout);

typedef struct __attribute__((__packed__)) node {
    unsigned int plainText;
    struct node* next;
}Node;



typedef uint8_t state_t[4][4];
void PrintState(state_t* s);
void PrintBuf(uint8_t *buf);
void build_key(unsigned int* partial_keys,uint8_t* key);
unsigned int bufToInt_Cipher(uint8_t *buf,int diagonal);
unsigned int bufToInt_Plain(const uint8_t *buf,int partial_diagonal);
void intToBuf_Plain(uint8_t *buf,int partial_diagonal,const unsigned int a);
int nthbit(unsigned int n, unsigned int* keyInvalid);
void setnthbit(unsigned int n, unsigned int* keyInvalid);
void write_valid_keys(unsigned long invalid_count, unsigned int *keyInvalid,int partial_diagonal);
void unsetnthbit(unsigned int n, unsigned int* keyInvalid);
int check_equal(uint8_t* b1,uint8_t* b2);
void read_hashtable(unsigned long* countarr,unsigned int* hasharr);
