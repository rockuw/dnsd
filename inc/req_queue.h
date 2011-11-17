#ifndef REQ_QUEUE
#define REQ_QUEUE

#include "dns_consts.h"
#include <arpa/inet.h>

#define REQ_QUEUE_SIZE 100

struct req_wrapper{
	struct sockaddr_in clnt_addr;
	uint8_t buffer[UDP_MSG_SIZE];
	int len;
};

struct req_queue{
	int in;
	int out;
	struct req_wrapper item_arr[REQ_QUEUE_SIZE];
};

int en_queue(struct req_queue *queue, struct sockaddr_in *addr, const uint8_t *buffer, int len);

int de_queue(struct req_queue *queue, struct req_wrapper *wrapper);
#endif
