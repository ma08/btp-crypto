#include "aes.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
int main(int argc, char *argv[])
{
    unsigned int* a;
    a=(unsigned int *)malloc(sizeof(unsigned int)*(1<<32));
    unsigned long i;
    for (i = 0; i < ((long)1)<<32; ++i)
    {
        a[i]=1;
    }
    return 0;
}
