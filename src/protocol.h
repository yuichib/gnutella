/*
 * protocol.h
 *
 *  Created on: 2009/04/24
 *      Author: yuichi
 */

//#define OSIP     "0.0.0.0"
//#define HOMEIP "219.110.174.127"
//#define LABIP  "192.168.23.52"
#define GNU_PORT 6346
#define HTTP_PORT 8080


#include <netinet/in.h>//typedef

#define GNUTELLA_CONNECT_MSG   "GNUTELLA CONNECT/0.4\n\n"
#define GNUTELLA_ACCEPT_MSG    "GNUTELLA OK\n\n"
#define GNUTELLA_ERROR_MSG     "GNUTELLA YOU ARE BAD CONNECT\n\n"

#define DESC_HEADER_SIZE 23//bytes
#define GUID_SIZE 16//bytes
#define SERVENT_ID_SIZE 16//bytes
#define PONG_DESC_SIZE 14//bytes
#define MAX_FILENAME_SIZE 128

//Descriptor Header
//Payload Descriptor
#define PING_ID 	0x00
#define PONG_ID 	0x01
#define PUSH_ID     0x40
#define QUERY_ID    0x80
#define QUERYHIT_ID 0x81


#define DEFAULT_TTL 7
#define MAX_TTL 100
#define INIT_HOP 0

#define OUTCOMING 0//from me
#define INCOMING  1//from peer



//Descriptor Header

typedef struct{

	uint8_t guid[GUID_SIZE];
	uint8_t payid;
	uint8_t ttl;
	uint8_t hops;
	uint32_t paylen;

}Desc_header;

//Ping Descriptor
//nothing

//Pong Descriptor
typedef struct{

	in_port_t port;//little endian
	in_addr_t ip;//big endian only
	uint32_t file_num;
	uint32_t kbyte_num;

}Pong_desc;

//Query Descriptor
typedef struct{

	uint16_t speed;
	char *criteria;

}Query_desc;

//Result Set
typedef struct{

	uint32_t file_index;
	uint32_t file_size;
	char file_name[MAX_FILENAME_SIZE];

}Result_set;

typedef struct{

	in_addr_t ip;
	in_port_t port;
	uint32_t speed;
	Result_set result_set;
	uint8_t servent_id[SERVENT_ID_SIZE];

}Result_setplus;

//Queryhit Descriptor
typedef struct{

	uint8_t hits;
	in_port_t port;
	in_addr_t ip;
	uint32_t speed;
	Result_set *result_set;
	uint8_t servent_id[SERVENT_ID_SIZE];

}Queryhit_desc;




typedef struct
{
	uint8_t guid[GUID_SIZE];
	int fd;//fromme or connection


}List;
