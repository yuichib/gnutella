/*
 * gnutella.c
 *
 *  Created on: 2009/04/24
 *      Author: yuichi
 */

#include <stdio.h>//perror
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>//send,recv
#include <sys/socket.h>
#include <sys/select.h>//select
#include <sys/time.h>//timeval
#include <netinet/in.h>//kata
#include "protocol.h"
#include "gnutella.h"
#include "mystr.h"

//local const
#define BUF_SIZE 128
#define MAX_QUEUE 5
#define TIMEOUT 3//sec

static void
Set_Error_Info(Info_payload *info)
{
	info->data = NULL;
	info->size = 0;

}

int
Gnutella_Create_Waitfd(struct sockaddr_in myskt)
{
	int fd;

	if((fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("socket");
		return -1;
	}
	if(bind(fd,(struct sockaddr *)&myskt,sizeof(myskt)) < 0 ){
		perror("bind");
		return -1;
	}
	if(listen(fd,MAX_QUEUE) < 0){
		perror("listen");
		return -1;
	}

	return fd;
}


int
Gnutella_Connect(int fd)
{
	int real_sendsize,real_rcvsize;
	int msglen;
	char sendbuf[BUF_SIZE],rcvbuf[BUF_SIZE];
	fd_set rdfds;
	struct timeval tv;
	int read_ok_num;


	strncpy(sendbuf,GNUTELLA_CONNECT_MSG,sizeof(sendbuf));
	msglen = strlen(sendbuf);
	if((real_sendsize = send(fd,sendbuf,msglen,0)) < 0){
		perror("send");
		return -1;
	}
	while(1){
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&rdfds);//init
		FD_SET(fd,&rdfds);//my connection
		if((read_ok_num = select(fd+1,&rdfds,NULL,NULL,&tv)) <= 0){//error or timeout
			perror("select");
			return -1;
		}
		else if(FD_ISSET(fd,&rdfds)){//my connection

			if((real_rcvsize = recv(fd,rcvbuf,sizeof(rcvbuf)-1,0)) < 0){
				perror("recv");
				return -1;
			}
			rcvbuf[real_rcvsize] = '\0';
			if(strcmp(rcvbuf,GNUTELLA_ACCEPT_MSG) == 0){
				return 1;//OK!!
			}

			return -1;//error message
		}
		else//other
			continue;
	}


}

int
Gnutella_Accept(int fd)
{
	int real_sendsize,real_rcvsize;
	int msglen;
	char sendbuf[BUF_SIZE],rcvbuf[BUF_SIZE];
	fd_set rdfds;
	struct timeval tv;
	int read_ok_num;

	while(1){
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&rdfds);//init
		FD_SET(fd,&rdfds);//my connection
		if((read_ok_num = select(fd+1,&rdfds,NULL,NULL,&tv)) <= 0){//error or timeout
			perror("select");
			return -1;
		}
		else if(FD_ISSET(fd,&rdfds)){//my connection

			if((real_rcvsize = recv(fd,rcvbuf,sizeof(rcvbuf)-1,0)) < 0){
				perror("recv");
				return -1;
			}
			break;
		}
		else//other
			continue;
	}

	rcvbuf[real_rcvsize] = '\0';
	if(strcmp(rcvbuf,GNUTELLA_CONNECT_MSG) == 0){//Correct MSG
		strncpy(sendbuf,GNUTELLA_ACCEPT_MSG,sizeof(sendbuf));
		msglen = strlen(sendbuf);
		if((real_sendsize = send(fd,sendbuf,msglen,0)) < 0){
			perror("send");
			return -1;
		}
		return 1;//OK
	}
	else{//Peer Bad Request
		strncpy(sendbuf,GNUTELLA_ERROR_MSG,sizeof(sendbuf));
		msglen = strlen(sendbuf);
		if((real_sendsize = send(fd,sendbuf,msglen,0)) < 0){
			perror("send");
			return -1;
		}
		return -1;
	}

}

int
Gnutella_Send(int fd,char *buf,int msglen)
{
	int real_sendsize;

	if((real_sendsize = send(fd,buf,msglen,0)) < 0){
		perror("send");
		return -1;
	}

	return real_sendsize;
}


int
Gnutella_Recv_Header(int fd,char *buf)
{
	int real_rcvsize,tmp_size = 0;
	char *tmp_ptr = buf;
	while(1){
		if(tmp_size >= DESC_HEADER_SIZE)
			break;
		if((real_rcvsize = recv(fd,tmp_ptr,(DESC_HEADER_SIZE - tmp_size), 0)) <= 0){
			if(real_rcvsize == 0)
				return 0;//success
			else{
				perror("recv");
				return -1;
			}
		}
		tmp_ptr += real_rcvsize;
		tmp_size += real_rcvsize;
	}
	real_rcvsize = tmp_size;

	return real_rcvsize;

}

int
Gnutella_Recv_Data(int fd,char *buf,int paylen)
{

	int real_rcvsize,tmp_size = 0;
	char *tmp_ptr = buf;
	while(1){
		if(tmp_size >= paylen)
			break;
		if((real_rcvsize = recv(fd,tmp_ptr,(paylen - tmp_size),0)) <= 0){
			if(real_rcvsize == 0)
				return 0;//success
			else{
				perror("recv");
				return -1;
			}
		}
		tmp_ptr += real_rcvsize;
		tmp_size += real_rcvsize;
	}
	real_rcvsize = tmp_size;

	return real_rcvsize;

}



void
Set_Desc_Header(Desc_header *desc_header,
		uint8_t *guid,uint8_t payid,uint8_t ttl,uint8_t hops,uint32_t paylen)
{

	memcpy(desc_header->guid,guid,GUID_SIZE);//16byte copy
	desc_header->payid = payid;
	desc_header->ttl = ttl;
	desc_header->hops = hops;
	desc_header->paylen = paylen;

}


void
Set_Pong_Desc(Pong_desc *pong_desc,
		in_port_t port,in_addr_t ip,uint32_t file_num,uint32_t kbyte_num)
{
	pong_desc->port = port;
	pong_desc->ip = ip;
	pong_desc->file_num = file_num;
	pong_desc->kbyte_num = kbyte_num;

}

void
Set_Query_Desc(Query_desc *query_desc,uint16_t speed,char *criteria)
{
	query_desc->speed = speed;
	query_desc->criteria = criteria;

}

void
Set_Queryhit_Desc(Queryhit_desc *queryhit_desc,
		uint8_t hits,in_port_t port,in_addr_t ip,uint32_t speed,Result_set *result_set,uint8_t *servent_id)
{
	queryhit_desc->hits = hits;
	queryhit_desc->port = port;
	queryhit_desc->ip = ip;
	queryhit_desc->speed = speed;
	queryhit_desc->result_set = result_set;
	memcpy(queryhit_desc->servent_id,servent_id,SERVENT_ID_SIZE);//16bytes copy

}

char *
Make_Desc_Header(Desc_header desc_header)
{

	char *ret,*pmal;
	if((pmal = (char *)malloc(DESC_HEADER_SIZE)) == NULL){
		return NULL;
	}
	ret = pmal;

	memcpy(pmal,desc_header.guid,GUID_SIZE);
	pmal += GUID_SIZE;//16bytes
	memcpy(pmal,&desc_header.payid,sizeof(desc_header.payid));
	pmal += sizeof(desc_header.payid);
	memcpy(pmal,&desc_header.ttl,sizeof(desc_header.ttl));
	pmal += sizeof(desc_header.ttl);
	memcpy(pmal,&desc_header.hops,sizeof(desc_header.hops));
	pmal += sizeof(desc_header.hops);
	memcpy(pmal,&desc_header.paylen,sizeof(desc_header.paylen));


	return ret;

}

Info_payload
Make_Ping(void)
{
	Info_payload info;
	info.data = NULL;
	info.size = 0;

	return info;
}

Info_payload
Make_Pong(Pong_desc pong_desc)
{
	Info_payload info;
	char *pmal;

	if((pmal = (char *)malloc(PONG_DESC_SIZE)) == NULL){
		Set_Error_Info(&info);
		return info;
	}
	info.data = pmal;
	info.size = PONG_DESC_SIZE;

	memcpy(pmal,&pong_desc.port,sizeof(pong_desc.port));
	pmal += sizeof(pong_desc.port);
	memcpy(pmal,&pong_desc.ip,sizeof(pong_desc.ip));//only big endian
	pmal += sizeof(pong_desc.ip);
	memcpy(pmal,&pong_desc.file_num,sizeof(pong_desc.file_num));
	pmal += sizeof(pong_desc.file_num);
	memcpy(pmal,&pong_desc.kbyte_num,sizeof(pong_desc.kbyte_num));

	return info;


}


Info_payload
Make_Query(Query_desc query_desc)
{
	Info_payload info;
	char *pmal;
	int len;

	len = strlen(query_desc.criteria) + 1;
	info.size = sizeof(query_desc.speed) + len;

	if((pmal = (char *)malloc(info.size)) == NULL){
		Set_Error_Info(&info);
		return info;
	}
	info.data = pmal;

	memcpy(pmal,&query_desc.speed,sizeof(query_desc.speed));
	pmal += sizeof(query_desc.speed);
	memcpy(pmal,query_desc.criteria,len);

	return info;

}

Info_payload
Make_Queryhit(Queryhit_desc queryhit_desc)
{
	Info_payload info;
	char *pmal;
	int i,len,allsize = 0;
	int hits = queryhit_desc.hits;

	allsize += sizeof(queryhit_desc.hits);
	allsize += sizeof(queryhit_desc.port);
	allsize += sizeof(queryhit_desc.ip);
	allsize += sizeof(queryhit_desc.speed);
	for(i=0; i<hits; i++){
		allsize += sizeof(queryhit_desc.result_set[i].file_index);
		allsize += sizeof(queryhit_desc.result_set[i].file_size);
		len = strlen(queryhit_desc.result_set[i].file_name) + 2;//double null 0x0000
		allsize += len;
	}
	allsize += sizeof(queryhit_desc.servent_id);

	if((pmal = (char *)malloc(allsize)) == NULL){
		Set_Error_Info(&info);
		return info;
	}
	info.data = pmal;
	info.size = allsize;

	memcpy(pmal,&queryhit_desc.hits,sizeof(queryhit_desc.hits));
	pmal += sizeof(queryhit_desc.hits);
	memcpy(pmal,&queryhit_desc.port,sizeof(queryhit_desc.port));
	pmal += sizeof(queryhit_desc.port);
	memcpy(pmal,&queryhit_desc.ip,sizeof(queryhit_desc.ip));
	pmal += sizeof(queryhit_desc.ip);
	memcpy(pmal,&queryhit_desc.speed,sizeof(queryhit_desc.speed));
	pmal += sizeof(queryhit_desc.speed);
	for(i=0; i<hits; i++){//result_set
		memcpy(pmal,&queryhit_desc.result_set[i].file_index,sizeof(queryhit_desc.result_set[i].file_index));
		pmal += sizeof(queryhit_desc.result_set[i].file_index);
		memcpy(pmal,&queryhit_desc.result_set[i].file_size,sizeof(queryhit_desc.result_set[i].file_size));
		pmal += sizeof(queryhit_desc.result_set[i].file_size);

		len = strlen(queryhit_desc.result_set[i].file_name);
		memcpy(pmal,queryhit_desc.result_set[i].file_name,len);
		pmal += len;
		*pmal = '\0';//add double null 0x0000
		pmal += 1;
		*pmal = '\0';
		pmal += 1;
	}
	memcpy(pmal,queryhit_desc.servent_id,sizeof(queryhit_desc.servent_id));

	return info;

}

void
Get_Desc_Header(char *buf,Desc_header *desc_header)
{
	char *p = buf;//copy

	memcpy(desc_header->guid,p,GUID_SIZE);
	p += GUID_SIZE;
	memcpy(&desc_header->payid,p,sizeof(desc_header->payid));
	p += sizeof(desc_header->payid);
	memcpy(&desc_header->ttl,p,sizeof(desc_header->ttl));
	p += sizeof(desc_header->ttl);
	memcpy(&desc_header->hops,p,sizeof(desc_header->hops));
	p += sizeof(desc_header->hops);
	memcpy(&desc_header->paylen,p,sizeof(desc_header->paylen));

}

void
Get_Ping(void)
{
	return;
}

void
Get_Pong(char *buf,Pong_desc *pong_desc)
{
	char *p = buf;//copy

	memcpy(&pong_desc->port,p,sizeof(pong_desc->port));
	p += sizeof(pong_desc->port);
	memcpy(&pong_desc->ip,p,sizeof(pong_desc->ip));
	p += sizeof(pong_desc->ip);
	memcpy(&pong_desc->file_num,p,sizeof(pong_desc->file_num));
	p += sizeof(pong_desc->file_num);
	memcpy(&pong_desc->kbyte_num,p,sizeof(pong_desc->kbyte_num));

}

void
Get_Query(char *buf,Query_desc *query_desc)
{
	char *p = buf;//copy

	memcpy(&query_desc->speed,p,sizeof(query_desc->speed));
	p += sizeof(query_desc->speed);
	query_desc->criteria = p;//pointer rcvdata


}

int
Get_Queryhit(char *buf,int bufsize,Queryhit_desc *queryhit_desc,Result_set *result_set,int max)
{

	char *p = buf;//copy
	queryhit_desc->result_set = result_set;//set result_set_rcv
	int i;
	int len;
	int real_hits;
	int real_bufsize = bufsize;

	memcpy(&queryhit_desc->hits,p,sizeof(queryhit_desc->hits));
	p += sizeof(queryhit_desc->hits);
	bufsize -= sizeof(queryhit_desc->hits);
	real_hits = queryhit_desc->hits;
	memcpy(&queryhit_desc->port,p,sizeof(queryhit_desc->port));
	p += sizeof(queryhit_desc->port);
	bufsize -= sizeof(queryhit_desc->port);
	memcpy(&queryhit_desc->ip,p,sizeof(queryhit_desc->ip));
	p += sizeof(queryhit_desc->ip);
	bufsize -= sizeof(queryhit_desc->ip);
	memcpy(&queryhit_desc->speed,p,sizeof(queryhit_desc->speed));
	p += sizeof(queryhit_desc->speed);
	bufsize -= sizeof(queryhit_desc->speed);
	for(i=0; i<real_hits; i++){//result_set
		if(i < max){
			memcpy(&result_set[i].file_index,p,sizeof(result_set[i].file_index));
			p += sizeof(result_set[i].file_index);
			bufsize -= sizeof(result_set[i].file_index);
			memcpy(&result_set[i].file_size,p,sizeof(result_set[i].file_size));
			p += sizeof(result_set[i].file_size);
			bufsize -= sizeof(result_set[i].file_size);

			if(Search_Mem(p,'\0',bufsize) == NULL){//Invalid format!!
				fprintf(stderr,"Search_Mem() error!\n");
				return -1;
			}
			len = strlen(p) + 2;//double null 0x0000
			if(len -1 > MAX_FILENAME_SIZE){
				fprintf(stderr,"Over Max Filename Size !\n");
				return -1;
			}
			strcpy(result_set[i].file_name,p);
			p += len;
		}
		else{
			queryhit_desc->hits = max;//update
			p = buf + real_bufsize - SERVENT_ID_SIZE;//for servent_id
			break;
		}
	}
	memcpy(queryhit_desc->servent_id,p,sizeof(queryhit_desc->servent_id));


	return 1;
}


/*
char *
Convert_Desc_Header(Desc_header desc_header)//caller must free
{
	cahr *ret,*pmal;
	if((pmal = malloc(DESC_HEADER_SIZE)) == NULL){
		return NULL;
	}
	ret = pmal;

	memcpy(pmal,&desc_header.desc_id[0],sizeof(desc_header.desc_id));
	pmal += sizeof(desc_header.desc_id);//16byte
	memcpy(pmal,&desc_header.pay_desc,sizeof(desc_header.pay_desc));
	pmal += sizeof(desc_header.pay_desc);
	memcpy(pmal,&desc_header.ttl,sizeof(desc_header.ttl));
	paml += sizeof(desc_header.ttl);
	memcpy(pmal,&desc_header.hops,sizeof(desc_header.hops));
	pmal += sizeof(desc_header.hops);
	memcpy(pmal,&desc_header.paylen,sizeof(desc_header.paylen));
	paml += sizeof(desc_header.paylen);

	return ret;

}

 */
