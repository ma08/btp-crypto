#include<stdio.h>
int main(int argc, char *argv[])
{
	unsigned char c[2] = {255,'\0'};
	printf("%s",c);
	printf("\n%d",c[0]);
	return 0;
}
