/*
 * myweb.c
 *
 *  Created on: 2009/04/08
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mystr.h"
#include "myweb.h"
#include "http.h"

#define STR_SIZE 256

void
Make_Status_Line(Status_Line status_line,char *buf)
{

	strcpy(buf,status_line.version);
	strcat(buf,SP);
	strcat(buf,status_line.code_and_reason);
	strcat(buf,CRLF);


}

void
Make_Object_Header(int filesize,char *buf){

	char str[STR_SIZE];
	snprintf(str,sizeof(str),"Content-Length: %d",filesize);
	strcat(buf,str);
	strcat(buf,CRLF);

}

void Make_End(char *buf){

	strcat(buf,CRLF);

}


void
Set_Default(Status_Line *status_line)
{
	status_line->version = HTTP_1_0;
	status_line->code_and_reason = OK;

}

void
Init_Var(char **argv,int argvsize,struct sockaddr_in *cliskt,socklen_t *cliskt_len)
{
	memset(argv,0,sizeof(char *) * argvsize);
	*cliskt_len = sizeof(struct sockaddr_in);
	memset(cliskt,0,*cliskt_len);

}

int
Check_Header_End(char *buf,int bufsize)
{
	int i;
	for(i=0; i<bufsize; i++,buf++){
		if(i + 3 < bufsize){
			if(*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r' && *(buf+3) == '\n'){//CRLF CRLF
				return 1;
			}
		}
	}

	return -1;

}


int
Check_Version(char *version)
{
	char *p;
	if( (p = Search_Char(version,'\r',strlen(version))) == NULL){
		return -1;
	}
	*p = '\0';
	if( (strcmp(version,HTTP_1_0) == 0) || (strcmp(version,HTTP_1_1) == 0) ){
		return 1;
	}
	else{
		return -1;
	}

}



//new


void
Make_Http_Get(char *buf,uint32_t index,char *filename)
{
	char tmp[64];
	snprintf(tmp,sizeof(tmp),"%d",(int)index);

	strcpy(buf,GET);
	strcat(buf,SP);
	strcat(buf,"/get/");
	strcat(buf,tmp);
	strcat(buf,"/");
	strcat(buf,filename);
	strcat(buf,"/");
	strcat(buf,SP);
	strcat(buf,HTTP_1_0);
	strcat(buf,CRLF);

}


int
Http_Recv_Header(int fd,char *buf,int bufsize)
{//until CRLFCRLF

	const int ONLY_ONE = 1;//1 byte zutu douzinikitemo ok
	int real_rcvsize,tmp_size = 0;
	char *tmp_ptr = buf;

	while(1){
		if(tmp_size >= bufsize){
			fprintf(stderr,"Can't Accept More because Buffer Size is %d bytes\n",bufsize);
			return -1;
		}
		if((real_rcvsize = recv(fd,tmp_ptr,ONLY_ONE,0)) <= 0){
			if(real_rcvsize == 0)
				return 0;//success
			else{
				perror("recv");
				return -1;
			}
		}
		tmp_ptr += real_rcvsize;
		tmp_size += real_rcvsize;
		if(Check_Header_End(buf,bufsize) > 0){//CRLF CRLF
			break;
		}
	}
	real_rcvsize = tmp_size;

	return real_rcvsize;


}



int
Rm_Slash(char *buf,int bufsize,uint32_t *index,char **filename)
{

	char *tmp;
	int len;
	char *p = buf;
	int size = bufsize;

	if(size <= 0)
		return -1;
	if(*p != '/')
		return -1;
	p++;
	size--;
	if((tmp = Search_Char(p,'/',size)) == NULL)
		return -1;
	*tmp = '\0';
	if(strcmp(p,"get") != 0)
		return -1;
	len = strlen("get") + 1;
	p += len;
	size -= len;
	if((tmp = Search_Char(p,'/',size)) == NULL)
		return -1;
	*tmp = '\0';
	*index = (uint32_t)atoi(p);
	len = (strlen(p) + 1);
	p += len;
	size -= len;
	if((tmp = Search_Char(p,'/',size)) == NULL)
		return -1;
	*tmp = '\0';
	*filename = p;

	return 1;


}


int
Get_Content_Length(char *buf,int bufsize)
{
	char *p = buf;
	int size = bufsize;

	int i;
	char *line;
	int len;
	int ret;
	char *head;
	char *tmp;

	for(i=1; (line = Get_Line(p,i,size))!=NULL; i++){
		len = strlen(line);

		if(strncmp(line,"Content-Length:",strlen("Content-Length:")) == 0 ||
				strncmp(line,"Content-length:",strlen("Content-Length:")) == 0){//Found
			head = line;
			head += strlen("Content-Length:");
			len  -= strlen("Content-Length:");
			if(len <= 0)
				return -1;
			while(*head == ' ' || *head == '\t'){//slip space
				head++;
				len--;
				if(len <= 0)
					return -1;
			}
			if((tmp = Search_Char(head,'\r',len)) == NULL)
				return -1;
			*tmp = '\0';
			ret = atoi(head);
			free(line);
			return ret;
		}

		free(line);
	}

	return -1;//Not Found


}







