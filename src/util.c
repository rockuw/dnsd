#include "util.h"
#include <stdio.h>

void print_msg(uint8_t *buffer, int len, const char *msg)
{
	int i, j;
	printf("------------------begin of %s------------------\n", msg);
	for(i = 0, j = 0; i < len; i ++){
		printf("%02x ", buffer[i]);
		if(j == 15){
			printf("\n");
			j = 0;
		} else {
			j ++;
		}
	}
	printf("\n------------------ end  of %s------------------\n", msg);
}
