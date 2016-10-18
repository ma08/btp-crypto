#include "aes.h"
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

void merge(){
    FILE * pFile, *hfile1,*hfile2;
    hfile1 = fopen ("hashtable1.bin", "r");
    hfile2 = fopen ("hashtable2.bin", "r");
    unsigned int *outarr=malloc(sizeof(int)*(long)1<<33);
    unsigned int count1,count2;
    unsigned long count;
    unsigned long i,j;
    pFile = fopen ("hashtable.bin", "wb");
    for(i=0;i<(long)1<<32;i++){
        fread(&count1,sizeof(unsigned int),1,hfile1);
        fread(&count2,sizeof(unsigned int),1,hfile2);
        fread(outarr+1,sizeof(unsigned int),count1,hfile1);
        fread(outarr+1+count1,sizeof(unsigned int),count2,hfile2);
        count=count1+count2;
        //printf("%lu %lu %lu %lu\n",count,count1,count2,i);
        //fflush(stdout);
        if(count>=(long)1<<32){
            printf("%lu %lu %lu %lu\n",count,count1,count2,i);
            fflush(stdout);
            break;
        }
        outarr[0]=count;
        fwrite (outarr , sizeof(int), count+1, pFile);
    }
    close(hfile1);
    close(hfile2);
    close(pFile);
}

int main(){
    merge();
    return 0;
}



