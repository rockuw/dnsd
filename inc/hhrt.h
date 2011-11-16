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

#endif
