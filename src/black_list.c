#include "black_list.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// djb2 hash function, see http://www.cse.yorku.ca/~oz/hash.html
int str_hash(const char *str)
{
	unsigned long hash = 5381;
	int c;
	while((c = *str++) != 0)
		hash = ((hash << 5) + hash) + c;
	
	return hash % BLIST_SIZE;
}

// probe
int hash_probe(int hash, int key)
{
	return (hash + key * key) % BLIST_SIZE;
}

// str matcher
int str_matcher(const char *domain, const char *entry)
{
	int i, j;
	for(i = strlen(domain) - 1, j = strlen(entry) - 1; i >= 0 && j >= 0 && domain[i] == entry[j]; i --, j --);
	if(i < 0 || j < 0) return 0;
	else return 1;
}

// blist lookup
int blist_lookup(struct black_list *blist, const char *domain)
{
	int h, i;
	struct blist_entry *entry;

	h = str_hash(domain);
	entry = blist->item_arr + h;

	// if the entry is invalid then record not exists
	if(entry->flag == 0) return -1;

	// find the right entry
	if(str_matcher(domain, entry->domain) == 0){
		return h;
	}

	i = 1;
	while(i < MAX_PROBE_TIMES &&entry->flag == 1){
		h = hash_probe(h, i);
		entry = blist->item_arr + h;
		if(str_matcher(domain, entry->domain) == 0){
			return h;
		}
		i ++;
	}
	// record not found
	if(entry->flag == 0){
		return -1;
	}
	if(i >= MAX_PROBE_TIMES){
		printf("probe failed!\n");
		exit(1);
	}
	return h;
}

// blist insert
int blist_insert(struct black_list *blist, const char *domain)
{
	int h, i;
	struct blist_entry *entry;

	h = str_hash(domain);
	entry = blist->item_arr + h;

	// the right place to insert
	if(entry->flag == 0){
		strcpy(entry->domain, domain);
		entry->flag = 1;
		blist->used ++;
		return h;
	}

	i = 1;
	while(i < MAX_PROBE_TIMES &&entry->flag == 1){
		h = hash_probe(h, i);
		entry = blist->item_arr + h;
		i ++;
	}
	// find the right place to insert
	if(entry->flag == 0){
		strcpy(entry->domain, domain);
		entry->flag = 1;
		blist->used ++;
	}
	// probe times exceeds threthold
	if(i >= MAX_PROBE_TIMES){
		printf("probe failed!\n");
		exit(1);
	}
	return h;
}

// init blist
int blist_init(struct black_list *blist)
{
	int i;
	struct blist_entry *entry;
	
	blist->used = 0;
	for(i = 0; i < BLIST_SIZE; i ++){
		entry = blist->item_arr + i;
		entry->flag = 0;
		entry->domain[0] = '\0';
	}
	return 0;
}
