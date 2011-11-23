#ifndef COMMON_H
#define COMMON_H

#define SHM_RQ_NAME "/req_shm"		// name for request queue
#define SHM_HHRT_NAME "/hhrt_shm"	// name for half handled request table
#define SHM_BLIST_NAME "/blist_shm"	// name for black list
#define SEM_MUTEX "/dnsd_sem_mutex"		// sem name for mutex
#define SEM_EMPTY "/dnsd_sem_empty"		// sem name for empty
#define SEM_FULL "/dnsd_sem_full"		// sem name for full

#define SEM_PSHARED 1				// for sem shared between processes

#define HIT_NS_SERVER "202.118.224.101" // hit DNS server

#endif
