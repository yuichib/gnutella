/*
 * myprint.c
 *
 *  Created on: 2009/04/25
 *      Author: yuichi
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>//inet_aton
#include <sys/types.h>
#include <sys/socket.h>
#include "protocol.h"


void
Print_Mem(char *ptr,int size)
{
	int i;
	for(i=0; i<size; i++){
		printf("%d:0x%x  ",i,*ptr);
		ptr++;
	}
	printf("\n");

}

void
Print_Desc_Header(Desc_header desc_header)
{
	int i;
	printf("+++ HEADER +++\n");
	printf("guid:0x");
	for(i=0; i<GUID_SIZE; i++){
		printf("%x",desc_header.guid[i]);
	}
	printf("\n");

	printf("payid:0x%x\n",desc_header.payid);
	printf("ttl:%d\n",desc_header.ttl);
	printf("hops:%d\n",desc_header.hops);
	printf("paylen:%d\n",(int)desc_header.paylen);
	printf("+++ HEADER END +++\n");
	printf("\n");

}

void
Print_Ping()
{
	return;
}

void
Print_Pong(Pong_desc pong_desc)
{
	struct in_addr tmp;
	tmp.s_addr = pong_desc.ip;

	printf("+++ PONG +++\n");
	printf("port:%d\n",pong_desc.port);
	printf("ip:%s\n",inet_ntoa(tmp));//inet_ntoa ha big endian de watasu
	printf("sharefile_num:%d\n",(int)pong_desc.file_num);
	printf("sharebyte_num:%d kbytes\n",(int)pong_desc.kbyte_num);
	printf("+++ PONG  END +++\n");

}

void
Print_Query(Query_desc query_desc)
{
	printf("+++ QUERY +++\n");
	printf("require minimum speed:%d\n",query_desc.speed);
	printf("search criteria:%s\n",query_desc.criteria);
	printf("+++ QUERY END +++\n");

}

void
Print_Queryhit(Queryhit_desc queryhit_desc)
{
	int i;
	struct in_addr tmp;
	tmp.s_addr = queryhit_desc.ip;

	printf("+++ QUERYHIT +++\n");
	printf("Number of Hits:%d\n",queryhit_desc.hits);
	printf("Wait HTTP port:%d\n",queryhit_desc.port);
	printf("Wait IP:%s\n",inet_ntoa(tmp));
	printf("Peer Speed:%d\n",(int)queryhit_desc.speed);
	printf("Result Set:\n");
	for(i=0; i<queryhit_desc.hits; i++){
		printf("index:%d size:%d name:%s\n",
				(int)queryhit_desc.result_set[i].file_index,
				(int)queryhit_desc.result_set[i].file_size,
				queryhit_desc.result_set[i].file_name);
	}
	printf("Servent Identifier:0x");
	for(i=0; i<SERVENT_ID_SIZE; i++){
		printf("%x",queryhit_desc.servent_id[i]);
	}
	printf("\n");
	printf("+++ QUERYHIT END +++\n");

}



void
Print_Direction(int direction)
{
	switch(direction){

	case OUTCOMING:
		printf("OUTCOMING\n");
		break;
	case INCOMING:
		printf("INCOMING\n");
		break;
	default:
		fprintf(stderr,"Invalid Direction\n");
		break;

	}

}
