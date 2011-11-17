#ifndef HHRT_H
#define HHRT_H

#include "dns_consts.h"
#include <arpa/inet.h>

#define HHRT_SIZE 100

struct hhrt_item{
	int old_id;
	struct sockaddr_in clnt_addr;
	int flag;
};

struct hhrt_table{
	int pos;
	struct hhrt_item item_arr[HHRT_SIZE];
};

int gen_hhrt_id(struct hhrt_table *hhrt);
int insert_hhrt(struct hhrt_table *hhrt, int pos, int o_id, struct sockaddr_in *c_addr);

int lookup_hhrt(struct hhrt_table *hhrt, int pos, struct hhrt_item *item);
#endif
