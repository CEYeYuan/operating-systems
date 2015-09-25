#include <stdio.h>
int main(int argc,char *argv[]){
	int i=1;
	printf("argc=%d\n",argc);
	for(int i=1;i<argc;i++){
		printf("%s\n",argv[i] );
	}
	return 0;
}
/*
./a.out to be
argc=3
to
be

*/