/*
   implement functions defined in protocol.h
   @rockuw
	Tue Oct 11 20:11:27 PDT 2011
*/
#include "protocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

// converto label format to text string
// arg: pointer to label, pointer to text
// ret: the converted label length
// eg: 01 61 03 63 6f 6d 00 -> a.com.

int label_to_text(const uint8_t *label, char *text)
{
	int oct_len, oct_offset, name_offset;

	oct_offset = 0;
	name_offset = 0;

	// should have some way out
	while(1){
		oct_len = *(label + oct_offset);
		oct_offset ++;
		if(oct_len == 0) break;
		memcpy(text + name_offset, label + oct_offset, oct_len);
		name_offset += oct_len;
		text[name_offset] = '.';
		name_offset ++;

		oct_offset += oct_len;
	}
	text[name_offset] = 0;

	return oct_offset;
}

// converto label format to text string, with msg compression supported
// arg: pointer to the entire buffer, pointer to label, pointer to text
// ret: the converted label length

int label_to_text2(const uint8_t *buffer, const uint8_t *label, char *text)
{
	int oct_len, oct_offset, name_offset, comp_offset;

	oct_offset = 0;
	name_offset = 0;

	// should have some way out
	while(1){
		oct_len = *(label + oct_offset);
		if((oct_len & 0xc0) == 0xc0){
			// uses msg compression
			comp_offset = (((oct_len & 0x3f) << 8) + *(label + oct_offset + 1));
			oct_offset += 2; // for the two octets offset
			label_to_text2(buffer, buffer + comp_offset, text + name_offset);
			break;
		} else {
			// a normal label
			oct_offset ++;
			if(oct_len == 0) break;
			memcpy(text + name_offset, label + oct_offset, oct_len);
			name_offset += oct_len;
			text[name_offset] = '.';
			name_offset ++;

			oct_offset += oct_len;
			text[name_offset] = 0;
		}
	}

	return oct_offset;
}

// converto text format to label
// arg: pointer to label, pointer to text
// ret: the converted label length
// eg: a.com. -> 01 61 03 63 6f 6d 00 

int text_to_label(const char *text, uint8_t *label)
{
	int i, oct_len, oct_offset;
	
	oct_len = 0;
	oct_offset = 0;

	for(i = 0; i < strlen(text); i ++){
		if(text[i] == '.'){
			label[oct_offset] = oct_len;
			oct_offset ++;
			memcpy(label + oct_offset, text + i - oct_len, oct_len);
			oct_offset += oct_len;
			oct_len = 0;
		} else {
			oct_len ++;
		}
	}
	label[oct_offset] = 0;
	oct_offset ++;

	return oct_offset;
}

// extract dns_query from buffer
// arg: pointer to the entire packet, the start of query buffer, buffer max len, pointer to query
// ret: dns_query length
int get_dns_query(const uint8_t *packet, const uint8_t *buffer, int len, struct dns_query *query)
{
	int oct_offset = 0;
	// get labels
	oct_offset += label_to_text2(packet, buffer + oct_offset, (char *)(query->qname));

	// get q type and q class
	query->qtype = ntohs(*((uint16_t *)(buffer + oct_offset)));
	oct_offset += 2; // for q type
	query->qclass = ntohs(*((uint16_t *)(buffer + oct_offset)));
	oct_offset += 2; // for q class
	return oct_offset;
}

// fill buffer with dns_query
// arg: pointer to dns_query, buffer, buffer max len
// ret: dns_query length
int set_dns_query(const struct dns_query *query, uint8_t *buffer, int len)
{
	int oct_offset = 0;
	// set labels
	oct_offset += text_to_label((char *)(query->qname), buffer + oct_offset);

	// set type
	*((uint16_t *)(buffer + oct_offset)) = htons(query->qtype);
	oct_offset += 2; // for q type
	// set class
	*((uint16_t *)(buffer + oct_offset)) = htons(query->qclass);
	oct_offset += 2; // for q class

	return oct_offset;
}

// extract msg header from buffer
// arg: buffer, buffer max len, pointer to header
// ret: status code
int get_dns_header(const uint8_t *buffer, int len, struct msg_header *header)
{
	struct msg_header *h = (struct msg_header *)buffer;
	header->id = ntohs(h->id);
	header->flag = ntohs(h->flag);
	header->qd_count = ntohs(h->qd_count);
	header->an_count = ntohs(h->an_count);
	header->ns_count = ntohs(h->ns_count);
	header->ar_count = ntohs(h->ar_count);

	return 0;
}

// fill buffer with msg_header
// arg: pointer to msg_header, buffer, buffer max len
// ret: status code
int set_dns_header(const struct msg_header *header, uint8_t *buffer, int len)
{
	struct msg_header *h = (struct msg_header *)buffer;
	h->id = htons(header->id);
	h->flag = htons(header->flag);
	h->qd_count = htons(header->qd_count);
	h->an_count = htons(header->an_count);
	h->ns_count = htons(header->ns_count);
	h->ar_count = htons(header->ar_count);
	return 0;
}

// fill buffer with rdata
// arg: pointer to rdata, buffer, pointer to buffer len
// ret: rdata length
int set_dns_rdata(const struct dns_rdata *rdata, uint8_t *buffer, int len)
{
	int oct_offset = 0;
	// set labels
	oct_offset += text_to_label((char *)(rdata->name), buffer + oct_offset);

	// set type
	*((uint16_t *)(buffer + oct_offset)) = htons(rdata->rtype);
	oct_offset += 2; // for r type
	// set class
	*((uint16_t *)(buffer + oct_offset)) = htons(rdata->rclass);
	oct_offset += 2; // for r class
	// set ttl
	*((uint32_t *)(buffer + oct_offset)) = htonl(rdata->ttl);
	oct_offset += 4; // for r ttl
	// set rd_length
	*((uint16_t *)(buffer + oct_offset)) = htons(rdata->rd_length);
	oct_offset += 2; // for rd_length

	memcpy(buffer + oct_offset, rdata->rdata, rdata->rd_length);
	oct_offset += rdata->rd_length;

	return oct_offset;
}

// extract rdata from buffer
// arg: pointer to the entire packet, the start of rdata buffer, buffer max len, pointer to rdata
// ret: the length of the rdata
int get_dns_rdata(const uint8_t *packet, const uint8_t *buffer, int len, struct dns_rdata *rdata)
{
	static uint8_t data_buffer[NAME_SIZE];
	static char name_buffer[NAME_SIZE];
	int tmp;

	int oct_offset = 0;
	// get labels
	oct_offset += label_to_text2(packet, buffer + oct_offset, (char *)(rdata->name));

	// get type and class
	rdata->rtype = ntohs(*((uint16_t *)(buffer + oct_offset)));
	oct_offset += 2; // for r type
	rdata->rclass = ntohs(*((uint16_t *)(buffer + oct_offset)));
	oct_offset += 2; // for r class

	// get ttl
	rdata->ttl = ntohl(*((uint32_t *)(buffer + oct_offset)));
	oct_offset += 4; // for ttl

	// get rdata length
	rdata->rd_length = ntohs(*((uint16_t *)(buffer + oct_offset)));
	oct_offset += 2; // for rdata length
	tmp = rdata->rd_length; // save the original length

	if(rdata->rtype == RTYPE_CNAME || rdata->rtype == RTYPE_NS || rdata->rtype == RTYPE_PTR){
		label_to_text2(packet, buffer + oct_offset, name_buffer); 
		rdata->rd_length = text_to_label(name_buffer, data_buffer);

		memcpy(rdata->rdata, data_buffer, rdata->rd_length);
		oct_offset += tmp;
	} else if (rdata->rtype == RTYPE_A){
		memcpy(rdata->rdata, buffer + oct_offset, rdata->rd_length);
		oct_offset += rdata->rd_length;
	} else if (rdata->rtype == RTYPE_MX) {
		printf("unknown rdata type: %d\n", rdata->rtype);
		return -1;
	} else if (rdata->rtype == RTYPE_TXT) {
		printf("unknown rdata type: %d\n", rdata->rtype);
		return -1;
	} else{
		printf("unknown rdata type: %d\n", rdata->rtype);
		return -1;
	}

	return oct_offset;
}

int get_msg_id(const uint8_t *buffer, int len)
{
	struct msg_header header;
	get_dns_header(buffer, len, &header);
	return header.id;
}

int verify_packet(const uint8_t *buffer, int len)
{
	// return code
	// -1: less than header size
	// -2: unkown op code
	// -3: msg is truncated
	// -4: Z code is not 0
	// -5: length > UDP_MSG_SIZE
	uint16_t flag;
	
	flag = ntohs(*((uint16_t *)(buffer + 2)));
	if(len < sizeof(struct msg_header)){
		return -11;
	}
	if(MSG_OPCODE(flag) > 2){
		return -2;
	}
	if(MSG_IS_TRUNCATED(flag)){
		return -3;
	}
	if(MSG_ZCODE(flag) != 0){
		return -4;
	}
	if(MSG_RCODE(flag) != 0){
		return MSG_RCODE(flag);
	}
	if(len > UDP_MSG_SIZE){
		return -5;
	}
	return 0;
}

int get_request(const uint8_t *buffer, int len, struct dns_msg *request)
{
	const uint8_t *question;
	int i, oct_offset;
	struct dns_query *cursor;

	// extract header
	get_dns_header(buffer, len, &request->header);

	// extract question
	oct_offset = 0;
	question = buffer + sizeof(struct msg_header);
	if(request->question != NULL) free(request->question);
	request->question = (struct dns_query *) malloc (sizeof(struct dns_query) * request->header.qd_count); // remember to free
	cursor = request->question;
	for(i = 0; i < request->header.qd_count; i ++, cursor ++){
		oct_offset += get_dns_query(buffer, question + oct_offset, 0, cursor);
	}

	return 0;
}

int set_request(const struct dns_msg *request, uint8_t *buffer, int *len)
{
	int i, oct_offset;
	uint8_t *question;
	struct dns_query *q_cursor;

	// set header
	set_dns_header(&request->header, buffer, 0);

	// set question
	question = buffer + sizeof(struct msg_header);
	oct_offset = 0;
	q_cursor = request->question;
	for(i = 0; i < request->header.qd_count; i ++, q_cursor ++){
		oct_offset += set_dns_query(q_cursor, question + oct_offset, 0);
	}
	*len = oct_offset + sizeof(struct msg_header);
	return 0;
}

int get_response(const uint8_t *buffer, int len, struct dns_msg *response)
{
	const uint8_t *question;
	int i, oct_offset;
	struct dns_query *q_cursor;
	struct dns_rdata *r_cursor;

	
	// extract header
	get_dns_header(buffer, len, &response->header);

	/* since the client didnot care about information in authority and additional section */
	response->header.ns_count = 0;
	response->header.ar_count = 0;

	// extract question
	oct_offset = 0;
	question = buffer + sizeof(struct msg_header);
	if(response->question != NULL) free(response->question);
	response->question = (struct dns_query *) malloc (sizeof(struct dns_query) * response->header.qd_count); // remember to free
	q_cursor = response->question;
	for(i = 0; i < response->header.qd_count; i ++, q_cursor ++){
		oct_offset += get_dns_query(buffer, question + oct_offset, 0, q_cursor);
	}

	// extract answer
	if(response->header.an_count !=0){
		if(response->answer != NULL) free(response->answer);
		response->answer = (struct dns_rdata *) malloc (sizeof(struct dns_rdata) * response->header.an_count);
		r_cursor = response->answer;
		for(i = 0; i < response->header.an_count; i ++, r_cursor ++){
			oct_offset += get_dns_rdata(buffer, question + oct_offset, 0, r_cursor);
		}
	}

	/* since the client didnot care about information in authority and additional section
	// extract authority
	if(response->header.ns_count !=0){
		if(response->authority != NULL) free(response->authority);
		response->authority = (struct dns_rdata *) malloc (sizeof(struct dns_rdata) * response->header.ns_count);
		r_cursor = response->authority;
		for(i = 0; i < response->header.ns_count; i ++, r_cursor ++){
			oct_offset += get_dns_rdata(buffer, question + oct_offset, 0, r_cursor);
		}
	}

	// extract additional
	if(response->header.ar_count !=0){
		if(response->additional != NULL) free(response->additional);
		response->additional = (struct dns_rdata *) malloc (sizeof(struct dns_rdata) * response->header.ar_count);
		r_cursor = response->additional;
		for(i = 0; i < response->header.ar_count; i ++, r_cursor ++){
			oct_offset += get_dns_rdata(buffer, question + oct_offset, 0, r_cursor);
		}
	}
	*/
	
	return 0;
}

int set_response(const struct dns_msg *response, uint8_t *buffer, int *len)
{
	//uint8_t *question;
	uint8_t *answer;
	struct dns_rdata *cursor;
	struct dns_query *q_cursor;
	int i, oct_offset;

	// set header
	set_dns_header(&response->header, buffer, 0);

	// set question
	answer = buffer + sizeof(struct msg_header);
	oct_offset = 0;
	q_cursor = response->question;
	for(i = 0; i < response->header.qd_count; i ++, q_cursor ++){
		oct_offset += set_dns_query(q_cursor, answer + oct_offset, 0);
	}

	// set answer
	cursor = response->answer;
	for(i = 0; i < response->header.an_count; i ++, cursor ++){
		oct_offset += set_dns_rdata(cursor, answer + oct_offset, 0);
	}

	// set authority
	cursor = response->authority;
	for(i = 0; i < response->header.ns_count; i ++, cursor ++){
		oct_offset += set_dns_rdata(cursor, answer + oct_offset, 0);
	}

	// set additional
	cursor = response->additional;
	for(i = 0; i < response->header.ar_count; i ++, cursor ++){
		oct_offset += set_dns_rdata(cursor, answer + oct_offset, 0);
	}

	// set authority
	// set additional

	*len = oct_offset + sizeof(struct msg_header);
	return 0;
}

// free memory allocated by malloc
int destroy_msg(struct dns_msg *msg)
{
	if(msg->question != NULL){
		free(msg->question);
	}
	if(msg->answer != NULL){
		free(msg->answer);
	}
	if(msg->authority != NULL){
		free(msg->authority);
	}
	if(msg->additional != NULL){
		free(msg->additional);
	}
	return 0;
}

