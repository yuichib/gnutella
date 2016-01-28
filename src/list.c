/*
 * list.c
 *
 *  Created on: 2009/05/02
 *      Author: yuichi
 */
#include <stdio.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>

#include "protocol.h"
#include "globalhandle.h"


int
Issue_Guid(uint8_t *guid)
{
	struct timeval tv;
	in_addr_t ip = Get_Myip();
	in_port_t portg = Get_Myportg();
	in_port_t porth = Get_Myporth();
	uint8_t *p = guid;

	if(gettimeofday(&tv,NULL) < 0){
		perror("gettimeofday");
		return -1;
	}

	memcpy(p,&ip,sizeof(ip));
	p += sizeof(ip);
	memcpy(p,&portg,sizeof(portg));
	p += sizeof(portg);
	memcpy(p,&porth,sizeof(porth));
	p += sizeof(porth);
	memcpy(p,&tv.tv_sec,4);
	p += 4;
	memcpy(p,&tv.tv_usec, 4);
	p += 4;

//	printf("%d %d %d %d %d", sizeof(ip), sizeof(portg), sizeof(porth), sizeof(tv.tv_sec), sizeof(tv.tv_usec));

	return 1;

}
