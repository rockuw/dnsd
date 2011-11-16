#include "req_queue.h"
#include "hhrt.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define RQ_SHM_KEY 1024		// key for request queue
#define HHRT_SHM_KEY 1102	// key for half handled request table

int g_skt;
struct sockaddr_in g_saddr;

void init_shm()
{
	// create shared memory for req queue
	if(shmget(RQ_SHM_KEY, sizeof(struct req_queue), IPC_CREAT | 0666) < 0){
		perror("shmget");
		exit(1);
	}

	// create shared memory for hhrt
	if(shmget(HHRT_SHM_KEY, sizeof(struct hhrt_item) * HHRT_SIZE, IPC_CREAT | 0666) < 0){
		perror("shmget");
		exit(1);
	}
}

void init_socket()
{
	memset((void *)&g_saddr, 0, sizeof(g_saddr));
	g_saddr.sin_family = AF_INET;
	g_saddr.sin_port = htons(DNS_PORT);
	g_saddr.sin_addr.g_saddr = INADDR_ANY;

	if((g_skt = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	
	if(bind(g_skt, (struct sockaddr *)&g_saddr, sizeof(struct sockaddr)) != 0){
		perror("bind");
		exit(1);
	}
}

int recver_entry()
{
	static uint8_t buffer[RECV_BUFFER_SIZE];
	struct sockaddr_in c_addr;
	int tmp;

	memset((void *)&c_addr, 0, sizeof(c_addr));

	while(1){
		// recv request
		fromlen = sizeof(struct sockaddr);
		len = recvfrom(g_skt, buffer, RECV_BUFFER_SIZE, 0, (struct sockaddr *)&c_addr, &fromlen);
		
		if((tmp = verify_packet(buffer, len)) < 0){
			printf("verify failed with code: %d\n", tmp);
			continue;
		}

		// put the buffer and client addr into req_queue
	}
}

int worker_entry()
{
}

int main()
{
	return 0;
}
