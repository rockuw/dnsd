#ifndef BLACK_LIST_H
#define BLACK_LIST_H

#define BLIST_SIZE	1000
#define MAX_PROBE_TIMES 100

#include "dns_consts.h"

struct blist_entry{
	int flag;
	char domain[NAME_SIZE];
};

struct black_list{
	int used;
	struct blist_entry item_arr[BLIST_SIZE];
};

// djb2 hash function, see http://www.cse.yorku.ca/~oz/hash.html
int str_hash(const char *str);

// probe function
int hash_probe(int hash, int key);

// str matcher
int str_matcher(const char *domain, const char *entry);

// blist lookup
int blist_lookup(struct black_list *blist, const char *domain);

// blist insert
int blist_insert(struct black_list *blist, const char *domain);

// init blist
int blist_init(struct black_list *blist);
#endif
