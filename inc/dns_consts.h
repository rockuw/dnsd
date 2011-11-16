/*
	define some consts in DNS protocol for example size limits
	@rockuw  
	Tue Oct 11 15:11:36 PDT 2011
*/

#ifndef DNS_CONSTS_H
#define DNS_CONSTS_H

#define LABLE_SIZE 64
#define NAME_SIZE 256
#define UDP_MSG_SIZE 512

#define DNS_PORT 53

enum e_record_type{
	RTYPE_A = 1,
	RTYPE_NS = 2,
	RTYPE_MD,
	RTYPE_MF,
	RTYPE_CNAME = 5,
	RTYPE_SOA,
	RTYPE_PTR = 12,
	RTYPE_MX = 15, 
	RTYPE_TXT = 16
};

#endif
