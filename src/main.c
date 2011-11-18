#include "req_queue.h"
#include "protocol.h"
#include "hhrt.h"
#include "util.h"
#include "black_list.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WORKER_NUMBER	3
#define RECV_BUFFER_SIZE 1024
#define SHM_RQ_NAME "/req_shm"		// name for request queue
#define SHM_HHRT_NAME "/hhrt_shm"	// name for half handled request table
#define SHM_BLIST_NAME "/blist_shm"	// name for black list
#define SEM_PSHARED 1				// for sem shared between processes
#define SEM_MUTEX "/dnsd_sem_mutex"		// sem name for mutex
#define SEM_EMPTY "/dnsd_sem_empty"		// sem name for empty
#define SEM_FULL "/dnsd_sem_full"		// sem name for full

int g_skt;
struct sockaddr_in g_saddr;

void init_shm()
{
	int shmid;
	// create shared memory for req queue
	if((shmid = shm_open(SHM_RQ_NAME, O_CREAT | O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if(ftruncate(shmid, sizeof(struct req_queue)) < 0){
		perror("ftruncate");
		exit(1);
	}
	// create shared memory for hhrt
	if((shmid = shm_open(SHM_HHRT_NAME, O_CREAT | O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if(ftruncate(shmid, sizeof(struct hhrt_table)) < 0){
		perror("ftruncate");
		exit(1);
	}
	// create shared memory for blist
	if((shmid = shm_open(SHM_BLIST_NAME, O_CREAT | O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if(ftruncate(shmid, sizeof(struct black_list)) < 0){
		perror("ftruncate");
		exit(1);
	}
}

void init_socket()
{
	memset((void *)&g_saddr, 0, sizeof(g_saddr));
	g_saddr.sin_family = AF_INET;
	g_saddr.sin_port = htons(DNS_PORT);
	g_saddr.sin_addr.s_addr = INADDR_ANY;

	if((g_skt = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	
	if(bind(g_skt, (struct sockaddr *)&g_saddr, sizeof(struct sockaddr)) != 0){
		perror("bind");
		exit(1);
	}
}

void init_sem()
{
	sem_t *sem_empty; 
	sem_t *sem_full;
	sem_t *sem_mutex;
	if((sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, REQ_QUEUE_SIZE)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
	if((sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
	if((sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
}

int init_req_queue()
{
	struct req_queue *queue;
	int shmid;

	if((shmid = shm_open(SHM_RQ_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((queue = mmap(NULL, sizeof(struct req_queue), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}
	queue->in = queue->out = 0;
	return 0;
}

int init_hhrt()
{
	struct hhrt_table *hhrt;
	int shmid;

	if((shmid = shm_open(SHM_HHRT_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((hhrt = mmap(NULL, sizeof(struct hhrt_table), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}
	hhrt->pos = 0;
	return 0;
}

int init_blist(const char *filename)
{
	struct black_list *blist;
	FILE *fp = NULL;
	int shmid;
	static char domain[NAME_SIZE];

	if((shmid = shm_open(SHM_BLIST_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((blist = mmap(NULL, sizeof(struct black_list), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}
	blist_init(blist);

	if((fp = fopen(filename, "r")) == NULL){
		perror("fopen");
		exit(1);
	}
	while(fscanf(fp, "%s", domain) == 1){
		blist_insert(blist, domain);
	}
	fclose(fp);
	return 0;
}

void *recver_entry(void *arg)
{
	static uint8_t buffer[RECV_BUFFER_SIZE];
	struct sockaddr_in c_addr;
	int tmp, shmid, len;
	struct req_queue *queue;
	socklen_t fromlen;
	sem_t *sem_empty, *sem_full;

	// debug
	printf("recver running...\n");

	// init semaphore
	if((sem_empty = sem_open(SEM_EMPTY, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
	if((sem_full = sem_open(SEM_FULL, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}

	// init shared memory
	if((shmid = shm_open(SHM_RQ_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((queue = mmap(NULL, sizeof(struct req_queue), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}

	while(1){
		// recv request
		fromlen = sizeof(struct sockaddr);
		len = recvfrom(g_skt, buffer, RECV_BUFFER_SIZE, 0, (struct sockaddr *)&c_addr, &fromlen);
		
		if((tmp = verify_packet(buffer, len)) < 0){
			printf("verify failed with code: %d\n", tmp);
			continue;
		}

		// put the buffer and client addr into req_queue
		sem_wait(sem_empty);
			en_queue(queue, &c_addr, buffer, len);
		sem_post(sem_full);
	}
}

void *worker_entry(void *arg)
{
	static struct req_wrapper wrapper;
	static struct hhrt_item hh_req;
	static struct sockaddr_in ns_addr;
	char *ns_server = "202.118.224.101";
	int shmid, hh_id, old_id;
	struct req_queue *queue;
	struct hhrt_table *hhrt;
	struct black_list *blist;
	sem_t *sem_empty, *sem_full, *sem_mutex;
	int worker_number = *((int *)arg);
	static uint8_t buffer[UDP_MSG_SIZE];
	static char domain[NAME_SIZE];
	int len;

	// debug
	printf("worker #%d running...\n", worker_number);

	// init NS addr
	memset((void *)&ns_addr, 0, sizeof(ns_addr));
	ns_addr.sin_family = AF_INET;
	ns_addr.sin_port = htons(DNS_PORT);
	inet_pton(AF_INET, ns_server, &ns_addr.sin_addr.s_addr);

	// init semaphore
	if((sem_empty = sem_open(SEM_EMPTY, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
	if((sem_full = sem_open(SEM_FULL, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}
	if((sem_mutex = sem_open(SEM_MUTEX, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(1);
	}

	// init shared memory
	if((shmid = shm_open(SHM_RQ_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((queue = mmap(NULL, sizeof(struct req_queue), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}

	if((shmid = shm_open(SHM_HHRT_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((hhrt = mmap(NULL, sizeof(struct hhrt_table), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}

	if((shmid = shm_open(SHM_BLIST_NAME, O_RDWR, 0666)) < 0){
		perror("shm_open");
		exit(1);
	}
	if((blist = mmap(NULL, sizeof(struct black_list), PROT_WRITE, MAP_SHARED, shmid, 0)) == (void *)(-1)){
		perror("mmap");
		exit(1);
	}

	while(1){
		sem_wait(sem_full);
			sem_wait(sem_mutex);
				de_queue(queue, &wrapper);		
				hh_id = gen_hhrt_id(hhrt);
			sem_post(sem_mutex);
		sem_post(sem_empty);

		if(msg_is_req(wrapper.buffer, wrapper.len)){
			// msg is a request
			old_id = get_msg_id(wrapper.buffer, wrapper.len);
			/*----------------------*/
				// add black list lookup
			get_msg_domain(wrapper.buffer, wrapper.len, domain);
			if(blist_lookup(blist, domain) == 0){
				len = make_error_resp(buffer, UDP_MSG_SIZE);
				set_msg_id(buffer, len, old_id);

				if(sendto(g_skt, buffer, len, 0, (struct sockaddr *)(&wrapper.clnt_addr), sizeof(struct sockaddr)) != len){
					perror("sendto");
				}
				continue;
			}

			/*----------------------*/
			insert_hhrt(hhrt, hh_id, old_id, &(wrapper.clnt_addr));
			set_msg_id(wrapper.buffer, wrapper.len, hh_id);
			// send to NS
			if(sendto(g_skt, wrapper.buffer, wrapper.len, 0, (struct sockaddr *)&ns_addr, sizeof(struct sockaddr)) != wrapper.len){
				perror("sendto");
				printf("to ns wrapper.len = %d\n", wrapper.len);
				continue;
			}
		} else {
			// msg is a response
			hh_id = get_msg_id(wrapper.buffer, wrapper.len);
			lookup_hhrt(hhrt, hh_id, &hh_req);
			set_msg_id(wrapper.buffer, wrapper.len, hh_req.old_id);

			// send to client
			if(sendto(g_skt, wrapper.buffer, wrapper.len, 0, (struct sockaddr *)&(hh_req.clnt_addr), sizeof(struct sockaddr)) != wrapper.len){
				perror("sendto");
				printf("to clnt wrapper.len = %d\n", wrapper.len);
				continue;
			}
		}
	}
}

int main()
{
	pthread_t recv_thread;
	pthread_t worker_threads[WORKER_NUMBER];
	int worker_numbers[WORKER_NUMBER];
	int i;

	init_socket();
	init_shm();
	init_sem();
	init_req_queue();
	init_hhrt();
	init_blist("black.list");
	
	pthread_create(&recv_thread, NULL, recver_entry, NULL);
	for(i = 0; i < WORKER_NUMBER; i ++){
		worker_numbers[i] = i;
		pthread_create(&worker_threads[i], NULL, worker_entry, &worker_numbers[i]);
	}

	pthread_join(recv_thread, NULL);
	for(i = 0; i < WORKER_NUMBER; i ++){
		pthread_join(worker_threads[i], NULL);
	}
	return 0;
}
