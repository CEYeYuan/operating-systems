#include "common.h"
#include "stdio.h"

int factorial(int input);
int main(int argc,char* argv[])
{
	//TBD();
	if(argc==1||argv[1][0]>'9'||argv[1][0]<'0')
		printf("%s\n","Huh?");
	else if(atoi(argv[1])>12)
		printf("%s\n","Overflow" );
	else 
		printf("%d\n",factorial(atoi(argv[1])));
	return 0;
}

int factorial(int input){
	if(input==1||input==0)
		return 1;
	else
		return input*factorial(input-1);
}
