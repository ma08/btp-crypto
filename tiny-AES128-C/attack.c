#include "aes.h"
#include<stdint.h>
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
                        /*PrintState(s1);*/
                        /*PrintState(s2);*/
                        InvCipherSinglePreComp(c1);
                        InvCipherSinglePreComp(c2);
                        count++;
                        if(count==(((long)1)<<3)){
                            printf("%lld",count);
                            return;
                        }
                        /*PrintState(s1);*/
                        /*PrintState(s2);*/

                        /*printf("%d %d %d %d",(*s1)[0][0]^(*s2)[0][0],(*s1)[1][1]^(*s2)[1][1],(*s1)[2][2]^(*s2)[2][2],(*s1)[3][3]^(*s2)[3][3]);*/
                        /*return;*/
                    }
}

int main(int argc, char *argv[])
{
    PreComputation();
    return 0;
}
