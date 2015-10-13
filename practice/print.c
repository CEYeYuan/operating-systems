#include <stdio.h>
int main(int argc, char  *argv[])
{
	/* code */
	printf("%p address of main\n", main);
	printf("%p address of argc\n", &argc);
	printf("%p address of argv\n", argv);
	printf("argv[0]:%s  ", argv[0]);
	printf("argc:%d  ", argc);
	printf("argc:%d  ", (int)(main[0]));
}