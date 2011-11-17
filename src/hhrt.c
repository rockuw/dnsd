#include "hhrt.h"
#include <string.h>

int gen_hhrt_id(struct hhrt_table *hhrt)
{
	hhrt->pos = (hhrt->pos + 1) % HHRT_SIZE;
	return hhrt->pos;
}

int insert_hhrt(struct hhrt_table *hhrt, int pos, int o_id, struct sockaddr_in *c_addr)
{
	struct hhrt_item *item = hhrt->item_arr +pos;
	item->old_id = o_id;
	item->flag = 1;
	memcpy(&(item->clnt_addr), c_addr, sizeof(struct sockaddr_in));
	return pos;
}

int lookup_hhrt(struct hhrt_table *hhrt, int pos, struct hhrt_item *item)
{
	struct hhrt_item *tmp = hhrt->item_arr +pos;
	memcpy(item, tmp, sizeof(struct hhrt_item));
	return pos;
}
