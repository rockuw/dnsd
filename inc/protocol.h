/*
   define DNS RR structures, request and response packet structures
   @rockuw
	Tue Oct 11 15:47:15 PDT 2011
*/
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "dns_consts.h"
#include <stdint.h>

struct msg_header{
	uint16_t id;
	uint16_t flag; // flag bits
	uint16_t qd_count;
	uint16_t an_count;
	uint16_t ns_count;
	uint16_t ar_count;
};

#define MSG_IS_QUERY(h) ((h & 0x8000) != 0)
#define MSG_OPCODE(h) ((h & 0x7800) >> 11)
#define MSG_IS_AUTH(h) ((h & 0x0400) != 0)
#define MSG_IS_TRUNCATED(h) ((h & 0x0200) != 0)
#define MSG_IS_RECUR_DESIRED(h) ((h & 0x0100) != 0)
#define MSG_IS_RECUR_AVAIL(h) ((h & 0x0080) != 0)
#define MSG_RCODE(h) ((h & 0x000f))
#define MSG_ZCODE(h) ((h & 0x0070) >> 4)

struct dns_query{
	uint8_t qname[NAME_SIZE];
	uint16_t qtype;
	uint16_t qclass;
};

struct dns_rdata{
	uint8_t name[NAME_SIZE];
	uint16_t rtype;
	uint16_t rclass;
	int32_t ttl;
	uint16_t rd_length;
	uint8_t rdata[NAME_SIZE];
};

struct dns_msg{
	struct msg_header header;
	struct dns_query *question;
	struct dns_rdata *answer;
	struct dns_rdata *authority;
	struct dns_rdata *additional;
};

// get request id
// arg: buffer, buffer len
// ret: id
int get_msg_id(const uint8_t *buffer, int len);

// set request id
// arg: buffer, buffer len, msg id
// ret: status code
int set_msg_id(uint8_t *buffer, int len, int id);

// determines whether the msg is a request
// arg: buffer, buffer len
// ret: 1 or 0
int msg_is_req(const uint8_t *buffer, int len);

// verify the packet is a valid dns message
// arg: buffer, buffer length
// ret: status code, 0 for OK, < 0 for sever error, > 0 warning
int verify_packet(const uint8_t *buffer, int len);

// extract request struct from buffer
// arg: buffer, buffer length, pointer to request
// ret: status code
int get_request(const uint8_t *buffer, int len, struct dns_msg *request);

// fill buffer with request struct
// arg: pointer to request struct, buffer, pointer to buffer length
// ret: status code
int set_request(const struct dns_msg *request, uint8_t *buffer, int *len);

// fill buffer with response struct
// arg: pointer to response struct, buffer, pointer to buffer length
// ret: status code
int set_response(const struct dns_msg *response, uint8_t *buffer, int *len);

// extract response from buffer
// arg: buffer, pointer to buffer length, pointer to response
// ret: status code
int get_response(const uint8_t *buffer, int len, struct dns_msg *response);

// destroy the msg that is no longer used, avoiding mem leak
// arg: pointer to the msg
// ret: status code
int destroy_msg(struct dns_msg *msg);

#endif
