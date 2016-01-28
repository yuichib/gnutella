/*
 * main.c
 *
 *  Created on: 2009/04/27
 *      Author: yuichi
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>//thread
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>//typedef
#include <arpa/inet.h>//inet_aton
#include <unistd.h>//close
#include <signal.h>//signal

#include "protocol.h"//order warning!!first!!
#include "gnutella.h"
#include "mystr.h"
#include "mycmd.h"
#include "mythread.h"
#include "globalhandle.h"
#include "getip.h"
#include "list.h"


//local const
#define STDIN_FD 0
#define CMD_SIZE 64
#define ARGV_SIZE 64
#define DEFAULT_DIR "../share/"
#define DEFAULT_MYSPEED 32000//kbps


//local struct
typedef struct{
	char *cmd;
	int (*pf)(int,char *[]);
}Cmd_tbl;

//local global
static Cmd_tbl cmdtbl[] = {
		{"connect",Cmd_Connect},
		{"ping",Cmd_Ping},
		{"query",Cmd_Query},
		{"push",Cmd_Push},
		{"get",Cmd_Get},
		{"cut",Cmd_Cut},
		{"initlist",Cmd_Initlist},
		{"netlist",Cmd_Netlist},
		{"help",Cmd_Help},
		{"exit",Cmd_Exit},
		{NULL,NULL}//end flag
};

/*****************global var******************************/
//thread
Th thtbl[MAX_THREAD_NUM];
//local_sharelist
char *share_dir;
int local_share_num;
Result_set local_sharelist[MAX_LOCALLIST_NUM];
//net_sharelist
Result_setplus net_sharelist[MAX_NETLIST_NUM];

//addr
in_addr_t myip;//big
in_port_t myportg;//little
in_port_t myporth;//little
//List
List ping_list[MAX_LIST_NUM];
List query_list[MAX_LIST_NUM];
//my state
uint16_t myspeed;

/*****************global var end******************************/




int
main(int argc,char *argv[],char *envp[])
{

	if(argc != 3 && argc != 4){
		fprintf(stderr,"%%./gnutella.exe gport hport or %%./gnutella.exe gport hport env\n");
		fprintf(stderr, "gport is the gnutella connection port number.\n");
		fprintf(stderr, "hport is the http connection port number.\n");
		fprintf(stderr, "env is your environment.\n");
		fprintf(stderr, "    0: Mac OS X with wired LAN(Default).\n");
		fprintf(stderr, "    1: Mac OS X with wireless LAN.\n");
		fprintf(stderr, "    2: Linux.\n");
		fprintf(stderr, "    3: Windows.\n");
		exit(-1);
	}
	/******************************************var****************************************/
	//net
	int gnufd;
	int httpfd;
	int newfd;
	//select
	fd_set rdfds;
	//tmp_aadr
	struct sockaddr_in myskt,peerskt;
	struct in_addr tmp;
	//socklen_t my_sktlen = sizeof(myskt);
	socklen_t peer_sktlen = sizeof(peerskt);
	//analyze
	char cmd[CMD_SIZE];
	Cmd_tbl *pcmd;
	char *p;
	int myargc;
	char *myargv[ARGV_SIZE];
	/******************************************var end****************************************/

	/*****************************************set addr*********************************/
	char *os = MAC_WIRED;
	if(argc >= 4){
		int env = atoi(argv[3]);
		if(env == 0) os = MAC_WIRED;
		if(env == 1) os = MAC_WIRELESS;
		if(env == 2) os = LINUX;
		if(env == 3) os = WIN;
	}
	myip = tmp.s_addr = GetMyIpAddr(os);
	memset(&myskt,0,sizeof(myskt));
	myskt.sin_addr.s_addr = Get_Myip();
	myskt.sin_family = AF_INET;
	//default
	myportg = GNU_PORT;
	myskt.sin_port = htons(GNU_PORT);//GNU port 6346(Big)
	if(argc >= 3){//%a.out port1 port2
		myportg = atoi(argv[1]);
		myskt.sin_port = htons(myportg);
	}

	if((gnufd = Gnutella_Create_Waitfd(myskt)) < 0){//Create Gnutella socket
		fprintf(stderr,"Cnutella_Create_Waitfd() error!\n");
		exit(-1);
	}
	//default
	myporth = HTTP_PORT;
	myskt.sin_port = htons(HTTP_PORT);//HTTP port 10010(Big)
	if(argc >= 3){//%a.out port1 port2
		myporth = atoi(argv[2]);
		myskt.sin_port = htons(myporth);
	}
	if((httpfd = Gnutella_Create_Waitfd(myskt)) < 0){//Create HTTP socket
		fprintf(stderr,"Cnutella_Create_Waitfd() error!\n");
		exit(-1);
	}
	//default
	share_dir = DEFAULT_DIR;
	/*****************************************set addr end*********************************/


	//init
	Init_Local_Sharelist(Get_Share_Dir());
	Init_Net_Sharelist();
	Init_Thtbl();
	Init_Ping_List();
	Init_Query_List();
	myspeed = DEFAULT_MYSPEED;

	//print
	printf("+++IP+++ %s\n",inet_ntoa(tmp));
	printf("+++gnuport+++ %d\n",Get_Myportg());
	printf("+++httpport+++ %d\n",Get_Myporth());
	printf("+++shared dir+++ %s\n",Get_Share_Dir());
	printf("+++shared file num+++ %d\n",Get_Local_Share_Num());
//	printf("+++responding speed+++ %d\n",Get_Myspeed());
	Print_Local_Sharelist();





	/*****************************while start*************************/
	while(1){
		fflush(stdin);
		fprintf(stderr,"[gnutella]%%");//prompt
		FD_ZERO(&rdfds);//init
		FD_SET(STDIN_FD,&rdfds);//keyboard
		FD_SET(gnufd,&rdfds);//gnutella
		FD_SET(httpfd,&rdfds);//http
		if(select(httpfd+1,&rdfds,NULL,NULL,NULL) < 0){//monitor & block
			perror("select");
			continue;
		}

		if(FD_ISSET(STDIN_FD,&rdfds)){//from keyboard,input cmd
			//analyze
			if(fgets(cmd,sizeof(cmd),stdin) == NULL){
				if(feof(stdin))
					fprintf(stderr,"EOF pushed\n");
				else
					fprintf(stderr,"fgets() error!\n");
				continue;
			}
			if((p = Search_Char(cmd,'\n',sizeof(cmd))) == NULL){
				fprintf(stderr,"Search_Char() error!\n");
				continue;
			}
			*p = '\0';
			getargs(cmd,&myargc,myargv);
			if(myargc <= 0)
				continue;
			//analyze end
			for(pcmd = cmdtbl; pcmd->cmd != NULL; pcmd++){
				if(strcmp(myargv[0],pcmd->cmd) == 0){
					if( (*pcmd->pf)(myargc,myargv) < 0){//cmd handler
						fprintf(stderr,"cmd %s error\n",pcmd->cmd);
					}
					break;
				}
			}
			if(pcmd->cmd == NULL)
				fprintf(stderr,"Unknown Command\n");

		}
		if(FD_ISSET(gnufd,&rdfds)){//from gnutella socket
			memset(&peerskt,0,sizeof(peerskt));
			peer_sktlen = sizeof(peerskt);
			if((newfd = accept(gnufd,(struct sockaddr *)&peerskt,&peer_sktlen)) < 0){
				perror("accept");
				continue;
			}
			if(Gnutella_Accept(newfd) < 0){//bad msg or timeout
				fprintf(stderr,"gnutella accept fail!!\n");
				close(newfd);
				continue;
			}
			printf("gnutella accept OK!!\n");
			/* create thread */
			if(Create_Thread(newfd,TYPE_GNU) < 0){//thread create error
				fprintf(stderr,"Create_Gnu_Thread() error!\n");
				close(newfd);
				continue;
			}
			printf("create thread success!!\n");

		}
		if(FD_ISSET(httpfd,&rdfds)){//from http socket
			printf("coming http port!\n");
			memset(&peerskt,0,sizeof(peerskt));
			peer_sktlen = sizeof(peerskt);
			if((newfd = accept(httpfd,(struct sockaddr *)&peerskt,&peer_sktlen)) < 0){
				perror("accept");
				continue;
			}
			/* create thread */
			if(Create_Thread(newfd,TYPE_HTTP) < 0){//thread create error
				fprintf(stderr,"Create_Http_Thread() error!\n");
				close(newfd);
				continue;
			}
			printf("create thread success!!\n");
		}
	}
	/*****************************while end*************************/




}
