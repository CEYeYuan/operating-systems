#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <unistd.h>
#include <assert.h>
#include "wc.h"

int
main(int argc, char *argv[])
{
	char *str=" fuck your mother you big shiit";
	struct wc *wc;
	wc=wc_init(str, 28l);
	printf("%p\n",wc );
	return 0;
}