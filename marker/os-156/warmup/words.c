#include "common.h"
#include "stdio.h"
int main(int argc,char*argv[])
{
	//TBD();
	int i;
	for(i=1;i<argc;i++){
		printf("%s\n",argv[i] );
	}
	return 0;
}
