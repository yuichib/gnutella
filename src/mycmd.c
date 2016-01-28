/*
 * mycmd.c
 *
 *  Created on: 2009/04/29
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>//gethostbyname
#include <unistd.h>//close
#include <pthread.h>

#include "protocol.h"
#include "mystr.h"
#include "gnutella.h"
#include "mythread.h"//Ttpe
#include "globalhandle.h"
#include "list.h"
#include "myweb.h"

//extern global
extern Th thtbl[];
//key
extern pthread_mutex_t key_thtbl;


int
Cmd_Connect(int argc,char *argv[])
{

	/*****************cmd connect start******************************/
	//%connect ip port
	if(argc != 3){
		fprintf(stderr,"format: %%connect ip port\n");
		return -1;
	}

	int createfd;
	struct sockaddr_in peerskt;
	struct hostent *peer_ent;
	socklen_t peer_sktlen = sizeof(peerskt);

	/* set peer addr */
	memset(&peerskt,0,sizeof(peerskt));
	peerskt.sin_family = AF_INET;
	if((peer_ent = gethostbyname(argv[1])) == NULL){
		herror("gethostbyname");
		return -1;
	}
	memcpy((char *)&peerskt.sin_addr,peer_ent->h_addr,peer_ent->h_length);
	peerskt.sin_port = htons(atoi(argv[2]));
	//create socket
	if((createfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("socket");
		return -1;
	}
	if(connect(createfd,(struct sockaddr *)&peerskt,peer_sktlen) < 0){//layer 4 connection
		perror("connect");
		close(createfd);
		return -1;
	}
	if(Gnutella_Connect(createfd) < 0){//don't accept peer(TIME OUT or BAD MESSAGE)
		fprintf(stderr,"gnutella connect fail!!\n");
		close(createfd);
		return -1;
	}
	printf("Gnutella Connect Ok!\n");
	/* create thread */
	if(Create_Thread(createfd,TYPE_GNU) < 0){//thread create error
		fprintf(stderr,"Create_Gnu_Thread() error!\n");
		close(createfd);
		return -1;
	}
	printf("create thread success!!\n");
	return 1;//OK!!
	/*****************cmd connect end******************************/

}

int
Cmd_Ping(int argc,char *argv[])//send all direct link node(type gnu)
{
	int i,fd;
	int real_sendsize;
	int errflag = 0;
	Desc_header desc_header;
	char *header;
	Info_payload info;
	char *sendbuf;
	//issue
	//set
	//Make
	//send

	//Data Setting
	Set_Desc_Header(&desc_header,
			DUMMY_GUID,PING_ID,DEFAULT_TTL,INIT_HOP,0);//set

	if(Issue_Guid(desc_header.guid) < 0){//issue
		fprintf(stderr,"Issue_Guid() error!\n");
		return -1;
	}

	Register_Ping_List(desc_header.guid,FROMME);//regist ping list
	info = Make_Ping();	//make payload
	desc_header.paylen = info.size;
	if((header = Make_Desc_Header(desc_header)) == NULL){//make header
		fprintf(stderr,"Make_Desc_Header() error!\n");
		return -1;
	}


	if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,info.data,info.size)) == NULL){//Cat_Data
		fprintf(stderr,"Cat_Data() error!\n");
		free(header);
		return -1;
	}
	//Data Setting End


	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){//for all gnutella direct connection
		if(thtbl[i].type == TYPE_GNU){//type gnutella connection
			fd = thtbl[i].fd;
			if((real_sendsize = Gnutella_Send(fd,sendbuf,(DESC_HEADER_SIZE+info.size))) < 0){//send
				fprintf(stderr,"Gnutella_Send() error!\n");
				errflag = 1;
			}
		}
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	//free
	if(header != NULL)
		free(header);
	if(info.data != NULL)
		free(info.data);
	if(sendbuf != NULL)
		free(sendbuf);

	if(errflag == 0)
		return 1;//OK
	else
		return -1;

}


int
Cmd_Query(int argc,char *argv[])//send all direct link node(type gnu)
{

	//%query speed criteria
	if(argc != 3){
		fprintf(stderr,"format: %%query speed(kbps) criteria\n");
		return -1;
	}

	int i,fd;
	int real_sendsize;
	int errflag = 0;
	Desc_header desc_header;
	char *header;
	Info_payload info;
	Query_desc query_desc;
	char *sendbuf;
	uint16_t speed = (uint16_t)atoi(argv[1]);
	char *criteria = argv[2];


	//Data Setting
	Set_Desc_Header(&desc_header,
			DUMMY_GUID,QUERY_ID,DEFAULT_TTL,INIT_HOP,0);//set
	if(Issue_Guid(desc_header.guid) < 0){//issue
		fprintf(stderr,"Issue_Guid() error!\n");
		return -1;
	}
	Register_Query_List(desc_header.guid,FROMME);//regist ping list

	Set_Query_Desc(&query_desc,speed,criteria);
	info = Make_Query(query_desc);	//make payload
	if(info.data == NULL){
		fprintf(stderr,"Make_Query() error!\n");
		return -1;
	}
	desc_header.paylen = info.size;
	if((header = Make_Desc_Header(desc_header)) == NULL){//make header
		fprintf(stderr,"Make_Desc_Header() error!\n");
		return -1;
	}
	if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,info.data,info.size)) == NULL){//Cat_Data
		fprintf(stderr,"Cat_Data() error!\n");
		free(header);
		return -1;
	}
	//Data Setting End


	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){//for all gnutella direct connection
		if(thtbl[i].type == TYPE_GNU){//type gnutella connection
			fd = thtbl[i].fd;
			if((real_sendsize = Gnutella_Send(fd,sendbuf,(DESC_HEADER_SIZE+info.size))) < 0){//send
				fprintf(stderr,"Gnutella_Send() error!\n");
				errflag = 1;
			}
		}
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	//free
	if(header != NULL)
		free(header);
	if(info.data != NULL)
		free(info.data);
	if(sendbuf != NULL)
		free(sendbuf);

	if(errflag == 0)
		return 1;//OK
	else
		return -1;


}

int
Cmd_Push(int argc,char *argv[])
{
	return -1;
}

/*
int
Cmd_Hconnect(int argc,char *argv[])
{

	//%hconnect ip port
	if(argc != 3){
		fprintf(stderr,"format: %%hconnect ip port\n");
		return -1;
	}

	int createfd;
	struct sockaddr_in peerskt;
	struct hostent *peer_ent;
	socklen_t peer_sktlen = sizeof(peerskt);

	// set peer addr
	memset(&peerskt,0,sizeof(peerskt));
	peerskt.sin_family = AF_INET;
	if((peer_ent = gethostbyname(argv[1])) == NULL){
		herror("gethostbyname");
		return -1;
	}
	memcpy((char *)&peerskt.sin_addr,peer_ent->h_addr,peer_ent->h_length);
	peerskt.sin_port = htons(atoi(argv[2]));
	//create socket
	if((createfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("socket");
		return -1;
	}
	if(connect(createfd,(struct sockaddr *)&peerskt,peer_sktlen) < 0){//layer 4 connection
		perror("connect");
		close(createfd);
		return -1;
	}
	printf("HTTP Connect Ok!\n");
	// create thread
	if(Create_Thread(createfd,TYPE_HTTP) < 0){//thread create error
		fprintf(stderr,"Create_Gnu_Thread() error!\n");
		close(createfd);
		return -1;
	}
	printf("create thread success!!\n");
	return 1;//OK!!

}
 */


int
Cmd_Get(int argc,char *argv[])
{

	if(argc != 2){
		fprintf(stderr,"%%format:%%get getid\n");
		return -1;
	}
	/*****************hconnect start******************************/

	int createfd;
	struct sockaddr_in peerskt;
	socklen_t peer_sktlen = sizeof(peerskt);
	Result_setplus result_setplus;
	int getid = atoi(argv[1]);
	int real_sendsize;
	int msglen;
	int errflag = 0;
	char sendbuf[1024];



	result_setplus = Get_Netlist_Resultplus(getid);
	if(result_setplus.result_set.file_size == 0){
		fprintf(stderr,"Not Found Such getid %d  in net_sharelist\n",getid);
		return -1;
	}


	/* set peer addr */
	memset(&peerskt,0,sizeof(peerskt));
	peerskt.sin_family = AF_INET;
	peerskt.sin_addr.s_addr = result_setplus.ip;
	peerskt.sin_port = htons(result_setplus.port);
	//create socket
	if((createfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("socket");
		return -1;
	}
	if(connect(createfd,(struct sockaddr *)&peerskt,peer_sktlen) < 0){//layer 4 connection
		perror("connect");
		close(createfd);
		return -1;
	}
	printf("HTTP Connect Ok!\n");
	/* create thread */
	if(Create_Thread(createfd,TYPE_HTTP) < 0){//thread create error
		fprintf(stderr,"Create_Gnu_Thread() error!\n");
		close(createfd);
		return -1;
	}
	printf("create thread success!!\n");
	/*****************hconnect end******************************/



	Make_Http_Get(sendbuf,result_setplus.result_set.file_index,result_setplus.result_set.file_name);
	/* if necessary
	 * 	append here
	 */
	Make_End(sendbuf);
	msglen = strlen(sendbuf);

	//lock
	pthread_mutex_lock(&key_thtbl);
	//send
	if((real_sendsize = Gnutella_Send(createfd,sendbuf,msglen)) < 0){
		fprintf(stderr,"Gnutella_Send() error!\n");
		errflag = 1;
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	if(errflag)
		return -1;
	else
		return 1;



}

int
Cmd_Cut(int argc,char *argv[])
{
	if(argc != 2){
		fprintf(stderr,"format:%%cut fd(connection)\n");
		return -1;
	}

	int fd = atoi(argv[1]);

	if(Kill_Thread(fd) < 0){
		fprintf(stderr,"Kill_Thread() error!\n");
		return -1;
	}

	return 1;//OK

}

int
Cmd_Initlist(int argc,char *argv[])
{
	Init_Ping_List();
	return 1;
}

int
Cmd_Netlist(int argc,char *argv[])
{


	Print_Net_Sharelist();


	return 1;

}


int
Cmd_Help(int argc,char *argv[])
{

	printf("\n");
	printf("##################################### help ####################################### \n");
	printf("+++ connect +++\n");
	printf("%%connect IP port\n");
	printf("establish gnutella connection with peer represented by (IP, port).\n");
	printf("-IP- is the peer's IP address. you can also specify domain name. ex)172.10.3.2, google.com \n");
	printf("-port- is the peer's Gnutella port number not http port number.  ex)6346 \n");
	printf("\n");

	printf("+++ ping +++\n");
	printf("%%ping\n");
	printf("send ping message to all connected peers. After sending ping, you may recv pong messages.\n");
	printf("\n");

	printf("+++ query +++\n");
	printf("%%query speed criteria\n");
	printf("send query message to all connected peers. After sending query, you may recv queryhit messages.\n");
	printf("-speed- is the minimum requirement bandwidth which you specify. ex) 100 (kbps)\n");
	printf("-criteria- is the part of file name which you want. ex) .mp3 \n");
	printf("\n");

	printf("+++ netlist +++\n");
	printf("%%netlist\n");
	printf("show the remote file list which you can download.\n");
	printf("\n");

	printf("+++ get +++\n");
	printf("%%get id\n");
	printf("download the remote file represented by id.\n");
	printf("-id- is the remote file id. you can get id from netlist command.\n");
	printf("\n");

	printf("+++ cut +++\n");
	printf("%%cut fd\n");
	printf("close the connection represented by fd.\n");
	printf("-fd- is the connection's file descriptor. \n");
	printf("\n");

	printf("+++ help +++\n");
	printf("%%help\n");
	printf("show this application's help.\n");
	printf("\n");

	printf("+++ exit +++\n");
	printf("%%exit\n");
	printf("shutdown this application.\n");
	printf("##################################### help end #################################### \n");
	printf("\n");

	//mainも



	fflush(stdout);


	return 1;
}

int
Cmd_Exit(int argc,char *argv[])
{
	printf("Bye!!\n");
	exit(0);

}

