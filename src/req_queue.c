#include "req_queue.h"
#include <string.h>

int en_queue(struct req_queue *queue, struct sockaddr_in *addr, const uint8_t *buffer, int len)
{
	struct req_wrapper *wrapper = queue->item_arr + queue->in;
	memcpy(&(wrapper->clnt_addr), addr, sizeof(struct sockaddr_in));
	memcpy((wrapper->buffer), buffer, len);
	wrapper->len = len;
	queue->in = (queue->in + 1) % REQ_QUEUE_SIZE;
	return 0;
}

int de_queue(struct req_queue *queue, struct req_wrapper *wrapper)
{
	memcpy(wrapper, &(queue->item_arr[queue->out]), sizeof(struct req_wrapper));
	queue->out = (queue->out + 1) % REQ_QUEUE_SIZE;
	return 0;
}
