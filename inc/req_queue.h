#ifndef REQ_QUEUE
#define REQ_QUEUE

#include "dns_consts.h"
#include <arpa/inet.h>

#define REQ_QUEUE_SIZE 100

struct req_wrapper{
	struct sockaddr_in clnt_addr;
	uint8_t buffer[UDP_MSG_SIZE];
};

struct req_queue{
	int in;
	int out;
	struct req_wrapper item_arr[REQ_QUEUE_SIZE];
};

#endif
