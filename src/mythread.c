/*
 * mythread.c
 *
 *  Created on: 2009/04/28
 *      Author: yuichi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "protocol.h"
#include "gnutella.h"
#include "mythread.h"
#include "globalhandle.h"
#include "myprint.h"
#include "mystr.h"
#include "myweb.h"
#include "myfile.h"
#include "http.h"


//extern global
extern Th thtbl[];
//extern key
extern pthread_mutex_t key_thtbl;
extern pthread_mutex_t key_print;

//local const
#define MAX_RESULT_NUM 100
#define DUMMY_SERVENT_ID (uint8_t *)"AAAABBBBCCCCDDDD"



//gnutella handle thread body
void *
Th_Gnufunc(void *arg)
{
	//soon die!!
	if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) != 0){
		fprintf(stderr,"pthread_setcanceltype() error!\n");
	}

	//thread
	int i;//for general
	int errflag;
	int fd = *(int *)arg;
	//int type = TYPE_GNU;
	int ret;//return Check_Ping_List() or Check_Query_List()
	//for recv & send
	Desc_header desc_header_rcv,desc_header_send;
	//Ping_desc Nothing!!
	Pong_desc pong_desc_rcv,pong_desc_send;
	Query_desc query_desc_rcv;
	Queryhit_desc queryhit_desc_rcv,queryhit_desc_send;
	int real_rcvsizeh;//header
	int real_rcvsized;//data
	int real_sendsize;
	char rcvhead_buf[DESC_HEADER_SIZE];//rcv header buffer
	char *rcvdata = NULL;//rcv data buffer ptr
	char *header = NULL;//send header buffer ptr
	Info_payload info;//send data buffer ptr & size
	char *sendbuf = NULL;//last send buffer ptr
	//for Query
	Result_set result_sets_rcv[MAX_RESULT_NUM],result_sets_send[MAX_RESULT_NUM];
	uint8_t hits;


	while(1){
		//init
		errflag = 0;
		/************************ COMMON RECV *******************************/

		if((real_rcvsizeh = Gnutella_Recv_Header(fd,rcvhead_buf)) <= 0){//recv header
			if(real_rcvsizeh == 0)
				fprintf(stderr,"Peer Shutdown Successfully\n");
			else
				fprintf(stderr,"Gnutella_Recv_Header() error!\n");
			End_Thread(pthread_self());//end
		}


		Get_Desc_Header(rcvhead_buf,&desc_header_rcv);//get header
		if((rcvdata = (char *)malloc(desc_header_rcv.paylen)) == NULL){//alloc payload buffer
			if(desc_header_rcv.payid != PING_ID){
				fprintf(stderr,"malloc() error invalid rcv!\n");
				End_Thread(pthread_self());//end
			}
		}
		if((real_rcvsized = Gnutella_Recv_Data(fd,rcvdata,desc_header_rcv.paylen)) <= 0){//recv payload
			if(desc_header_rcv.payid != PING_ID){//ping always return 0
				if(real_rcvsized == 0)
					fprintf(stderr,"Peer Shutdown Successfully\n");
				else
					fprintf(stderr,"Gnutella_Recv_Data() error!\n");
				End_Thread(pthread_self());//end
			}
		}
		switch(desc_header_rcv.payid){//get data
		case PING_ID:
			Get_Ping();//nothing
			break;
		case PONG_ID:
			Get_Pong(rcvdata,&pong_desc_rcv);
			break;
		case QUERY_ID:
			Get_Query(rcvdata,&query_desc_rcv);
			if(query_desc_rcv.criteria[desc_header_rcv.paylen - sizeof(query_desc_rcv.speed) -1] != '\0'){//NULL END?
				fprintf(stderr,"Criteria Not NULL End!\n");
				free(rcvdata);
				End_Thread(pthread_self());//end
			}
			break;
		case QUERYHIT_ID:
			if(Get_Queryhit(rcvdata,desc_header_rcv.paylen,
					&queryhit_desc_rcv,result_sets_rcv,MAX_RESULT_NUM) < 0){
				fprintf(stderr,"Result set Not double NULL End!\n");
				free(rcvdata);
				End_Thread(pthread_self());//end
			}
			break;
		default:
			fprintf(stderr,"Unkown Payid %d\n",desc_header_rcv.payid);
			break;
		}
		/************************COMMON RECV END****************************/




		/************************************** Reply & Rely ******************************/
		/*********CASE:PING********/
		if(desc_header_rcv.payid == PING_ID){

			ret = Check_Ping_List(desc_header_rcv.guid);
			if(ret >= 0){//already see ping!!
				printf("already seen ping\n");
				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//do nothing
			}

			//first see ping
			Register_Ping_List(desc_header_rcv.guid,fd);

			/************************Print Head & Data********************************/
			//lock
			pthread_mutex_lock(&key_print);
			printf("\n---------------------------------------------------------\n");
			printf("connection %d recv header %d bytes \n",fd,real_rcvsizeh);//waning!!
			Print_Desc_Header(desc_header_rcv);//print
			printf("connection %d recv data %d bytes\n",fd,real_rcvsized);
			switch(desc_header_rcv.payid){//print data
			case PING_ID:
				Print_Ping();//nothing
				break;
			case PONG_ID:
				Print_Pong(pong_desc_rcv);
				break;
			default:
				break;
			}
			printf("----------------------------------------------------------\n");
			//unlock
			pthread_mutex_unlock(&key_print);
			/************************Print Head & Data end********************************/



			/*****************Reply****************/
			Set_Desc_Header(&desc_header_send,
					desc_header_rcv.guid,PONG_ID,DEFAULT_TTL,INIT_HOP,0);//set
			Set_Pong_Desc(&pong_desc_send,
					Get_Myportg(),Get_Myip(),Get_Sharefile_Num(),Get_Sharebyte_Num());
			info = Make_Pong(pong_desc_send);	//make payload
			if(info.data == NULL){
				fprintf(stderr,"Make_Pong() error!\n");
				End_Thread(pthread_self());//end
			}
			desc_header_send.paylen = info.size;
			if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
				fprintf(stderr,"Make_Desc_Header() error!\n");
				free(info.data);
				End_Thread(pthread_self());//end
			}
			if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,info.data,info.size)) == NULL){//Cat_Data
				fprintf(stderr,"Cat_Data() error!\n");
				free(info.data);
				free(header);
				End_Thread(pthread_self());//end
			}

			//lock for send pong
			pthread_mutex_lock(&key_thtbl);
			//for my connection
			if((real_sendsize = Gnutella_Send(fd,sendbuf,(DESC_HEADER_SIZE+info.size))) < 0){//send
				fprintf(stderr,"Gnutella_Send() error!\n");
				errflag = 1;
			}
			//unlock
			pthread_mutex_unlock(&key_thtbl);

			//free pong data
			if(header != NULL)
				free(header);
			if(info.data != NULL)
				free(info.data);
			if(sendbuf != NULL)
				free(sendbuf);
			//error?
			if(errflag)
				End_Thread(pthread_self());//end
			/****************Reply End*************/

			//ttl--,hops++
			desc_header_rcv.ttl--;
			desc_header_rcv.hops++;
			if(desc_header_rcv.ttl <= 0){//kill
				if(rcvdata != NULL)
					free(rcvdata);
				continue;
			}
			if(desc_header_rcv.ttl >= MAX_TTL){//much larger!!
				desc_header_rcv.ttl = MAX_TTL;
			}

			/*****************Rely****************/
			Set_Desc_Header(&desc_header_send,
					desc_header_rcv.guid,desc_header_rcv.payid,desc_header_rcv.ttl,
					desc_header_rcv.hops,desc_header_rcv.paylen);//set

			if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
				fprintf(stderr,"Make_Desc_Header() error!\n");
				End_Thread(pthread_self());//end
			}
			if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,rcvdata,(int)desc_header_send.paylen)) == NULL){//Cat_Data
				fprintf(stderr,"Cat_Data() error!\n");
				free(header);
				End_Thread(pthread_self());//end
			}

			//lock
			pthread_mutex_lock(&key_thtbl);
			for(i=0; i<MAX_THREAD_NUM; i++){//for all gnutella direct connection except my connection and http
				if(thtbl[i].type == TYPE_GNU && thtbl[i].fd != fd){
					if((real_sendsize = Gnutella_Send(thtbl[i].fd,sendbuf,(DESC_HEADER_SIZE+(int)desc_header_send.paylen))) < 0){//send
						fprintf(stderr,"Gnutella_Send() error!\n");
						errflag = 1;
					}
				}
			}
			//unlock
			pthread_mutex_unlock(&key_thtbl);


			/*****************Rely End************/

			//free
			if(rcvdata != NULL)
				free(rcvdata);
			if(header != NULL)
				free(header);
			if(sendbuf != NULL)
				free(sendbuf);
			//error?
			if(errflag)
				End_Thread(pthread_self());//end

		}
		/*********CASE:PING END********/

		/*********CASE:PONG********/
		else if(desc_header_rcv.payid == PONG_ID){

			ret = Check_Ping_List(desc_header_rcv.guid);
			if(ret <  0){//not seen such  pong<->ping!!
				fprintf(stderr,"I haven't seen such pong\n");
				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//do nothing
			}
			else if(ret == 0){//FROMME
				/************************Print Head & Data********************************/
				//lock
				pthread_mutex_lock(&key_print);
				printf("\n---------------------------------------------------------\n");
				printf("connection %d recv header %d bytes \n",fd,real_rcvsizeh);//waning!!
				Print_Desc_Header(desc_header_rcv);//print
				printf("connection %d recv data %d bytes\n",fd,real_rcvsized);
				Print_Pong(pong_desc_rcv);
				printf("----------------------------------------------------------\n");
				//unlock
				pthread_mutex_unlock(&key_print);
				/************************Print Head & Data end********************************/
				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//print & end
			}
			else{//from other node , send only one(not fd!)

				//ttl--,hops++
				desc_header_rcv.ttl--;
				desc_header_rcv.hops++;
				if(desc_header_rcv.ttl <= 0){//kill
					if(rcvdata != NULL)
						free(rcvdata);
					continue;
				}
				if(desc_header_rcv.ttl >= MAX_TTL){//much larger!!
					desc_header_rcv.ttl = MAX_TTL;
				}

				/*****************Rely****************/
				Set_Desc_Header(&desc_header_send,
						desc_header_rcv.guid,desc_header_rcv.payid,desc_header_rcv.ttl,
						desc_header_rcv.hops,desc_header_rcv.paylen);//set

				if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
					fprintf(stderr,"Make_Desc_Header() error!\n");
					End_Thread(pthread_self());//end
				}
				if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,
						rcvdata,(int)desc_header_rcv.paylen)) == NULL){//Cat_Data
					fprintf(stderr,"Cat_Data() error!\n");
					free(header);
					End_Thread(pthread_self());//end
				}

				//lock (for send)
				pthread_mutex_lock(&key_thtbl);
				if((real_sendsize = Gnutella_Send(ret,sendbuf,
						(DESC_HEADER_SIZE+(int)desc_header_rcv.paylen))) < 0){//send
					fprintf(stderr,"Gnutella_Send() error!\n");
					errflag = 1;
				}
				//unlock
				pthread_mutex_unlock(&key_thtbl);

				/*****************Rely End************/

				//free
				if(rcvdata != NULL)
					free(rcvdata);
				if(header != NULL)
					free(header);
				if(sendbuf != NULL)
					free(sendbuf);
				//error?
				if(errflag)
					End_Thread(pthread_self());//end
			}//else end

		}
		/*********CASE:PONG END********/
		/*********CASE:QUERY********/
		else if(desc_header_rcv.payid == QUERY_ID){

			ret = Check_Query_List(desc_header_rcv.guid);
			if(ret >= 0){//already see query!!
				printf("already seen query\n");
				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//do nothing
			}

			//first see query
			Register_Query_List(desc_header_rcv.guid,fd);


			/************************Print Head & Data********************************/
			//lock
			pthread_mutex_lock(&key_print);
			printf("\n---------------------------------------------------------\n");
			printf("connection %d recv header %d bytes \n",fd,real_rcvsizeh);
			Print_Desc_Header(desc_header_rcv);//print
			printf("connection %d recv data %d bytes\n",fd,real_rcvsized);
			Print_Query(query_desc_rcv);
			printf("----------------------------------------------------------\n");
			//unlock
			pthread_mutex_unlock(&key_print);
			/************************Print Head & Data end********************************/

			//search local share list
			Search_Local_Sharelist(query_desc_rcv.criteria,result_sets_send,MAX_RESULT_NUM,&hits);
			/*printf("local hits %d\n",hits);
			for(i=0; i<hits; i++){
				printf("id:%d size:%d (bytes) name:%s\n",
						(int)result_sets_send[i].file_index,(int)result_sets_send[i].file_size,result_sets_send[i].file_name);
			}
			 */

			/*****************Reply****************/
			if( (hits > 0) && (Get_Myspeed() >= query_desc_rcv.speed) ){//reply queryhit

				Set_Desc_Header(&desc_header_send,
						desc_header_rcv.guid,QUERYHIT_ID,DEFAULT_TTL,INIT_HOP,0);//set
				Set_Queryhit_Desc(&queryhit_desc_send,
						hits,Get_Myporth(),Get_Myip(),Get_Myspeed(),
						result_sets_send,DUMMY_SERVENT_ID);
				Set_My_Servent_Id(queryhit_desc_send.servent_id);

				info = Make_Queryhit(queryhit_desc_send);	//make payload
				if(info.data == NULL){
					fprintf(stderr,"Make_Queryhit() error!\n");
					End_Thread(pthread_self());//end
				}
				desc_header_send.paylen = info.size;
				if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
					fprintf(stderr,"Make_Desc_Header() error!\n");
					free(info.data);
					End_Thread(pthread_self());//end
				}
				if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,info.data,info.size)) == NULL){//Cat_Data
					fprintf(stderr,"Cat_Data() error!\n");
					free(info.data);
					free(header);
					End_Thread(pthread_self());//end
				}

				//lock for send queryhit
				pthread_mutex_lock(&key_thtbl);
				//for my connection
				if((real_sendsize = Gnutella_Send(fd,sendbuf,(DESC_HEADER_SIZE+info.size))) < 0){//send
					fprintf(stderr,"Gnutella_Send() error!\n");
					errflag = 1;
				}
				//unlock
				pthread_mutex_unlock(&key_thtbl);

				//free queryhit data
				if(header != NULL)
					free(header);
				if(info.data != NULL)
					free(info.data);
				if(sendbuf != NULL)
					free(sendbuf);
				//error?
				if(errflag)
					End_Thread(pthread_self());//end

			}
			/*****************Reply End****************/

			//ttl--,hops++
			desc_header_rcv.ttl--;
			desc_header_rcv.hops++;
			if(desc_header_rcv.ttl <= 0){//kill
				if(rcvdata != NULL)
					free(rcvdata);
				continue;
			}
			if(desc_header_rcv.ttl >= MAX_TTL){//much larger!!
				desc_header_rcv.ttl = MAX_TTL;
			}

			/*****************Rely****************/
			Set_Desc_Header(&desc_header_send,
					desc_header_rcv.guid,desc_header_rcv.payid,desc_header_rcv.ttl,
					desc_header_rcv.hops,desc_header_rcv.paylen);//set

			if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
				fprintf(stderr,"Make_Desc_Header() error!\n");
				End_Thread(pthread_self());//end
			}
			if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,rcvdata,(int)desc_header_send.paylen)) == NULL){//Cat_Data
				fprintf(stderr,"Cat_Data() error!\n");
				free(header);
				End_Thread(pthread_self());//end
			}

			//lock
			pthread_mutex_lock(&key_thtbl);
			for(i=0; i<MAX_THREAD_NUM; i++){//for all gnutella direct connection except my connection and http
				if(thtbl[i].type == TYPE_GNU && thtbl[i].fd != fd){
					if((real_sendsize = Gnutella_Send(thtbl[i].fd,sendbuf,(DESC_HEADER_SIZE+(int)desc_header_send.paylen))) < 0){//send
						fprintf(stderr,"Gnutella_Send() error!\n");
						errflag = 1;
					}
				}
			}
			//unlock
			pthread_mutex_unlock(&key_thtbl);


			/*****************Rely End************/

			//free
			if(rcvdata != NULL)
				free(rcvdata);
			if(header != NULL)
				free(header);
			if(sendbuf != NULL)
				free(sendbuf);
			//error?
			if(errflag)
				End_Thread(pthread_self());//end


		}
		/*********CASE:QUERY END********/
		/*********CASE:QUERYHIT********/
		else if(desc_header_rcv.payid == QUERYHIT_ID){

			ret = Check_Query_List(desc_header_rcv.guid);
			if(ret <  0){//not seen such  query<->queryhit!!
				fprintf(stderr,"I haven't seen such queryhit\n");
				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//do nothing
			}
			else if(ret == 0){//FROMME

				/************************Print Head & Data********************************/
				//lock
				pthread_mutex_lock(&key_print);
				printf("\n---------------------------------------------------------\n");
				printf("connection %d recv header %d bytes \n",fd,real_rcvsizeh);//waning!!
				Print_Desc_Header(desc_header_rcv);//print
				printf("connection %d recv data %d bytes\n",fd,real_rcvsized);
				Print_Queryhit(queryhit_desc_rcv);
				printf("----------------------------------------------------------\n");
				//unlock
				pthread_mutex_unlock(&key_print);
				/************************Print Head & Data end********************************/

				//register_net_sharelist
				Register_Net_Sharelist(queryhit_desc_rcv);

				//free
				if(rcvdata != NULL)
					free(rcvdata);
				continue;//print & end
			}
			else{//from other node , send only one(not fd!)

				//ttl--,hops++
				desc_header_rcv.ttl--;
				desc_header_rcv.hops++;
				if(desc_header_rcv.ttl <= 0){//kill
					if(rcvdata != NULL)
						free(rcvdata);
					continue;
				}
				if(desc_header_rcv.ttl >= MAX_TTL){//much larger!!
					desc_header_rcv.ttl = MAX_TTL;
				}

				/*****************Rely****************/
				Set_Desc_Header(&desc_header_send,
						desc_header_rcv.guid,desc_header_rcv.payid,desc_header_rcv.ttl,
						desc_header_rcv.hops,desc_header_rcv.paylen);//set

				if((header = Make_Desc_Header(desc_header_send)) == NULL){//make header
					fprintf(stderr,"Make_Desc_Header() error!\n");
					End_Thread(pthread_self());//end
				}
				if((sendbuf = Cat_Data(header,DESC_HEADER_SIZE,
						rcvdata,(int)desc_header_rcv.paylen)) == NULL){//Cat_Data
					fprintf(stderr,"Cat_Data() error!\n");
					free(header);
					End_Thread(pthread_self());//end
				}

				//lock (for send)
				pthread_mutex_lock(&key_thtbl);
				if((real_sendsize = Gnutella_Send(ret,sendbuf,
						(DESC_HEADER_SIZE+(int)desc_header_rcv.paylen))) < 0){//send
					fprintf(stderr,"Gnutella_Send() error!\n");
					errflag = 1;
				}
				//unlock
				pthread_mutex_unlock(&key_thtbl);

				/*****************Rely End************/

				//free
				if(rcvdata != NULL)
					free(rcvdata);
				if(header != NULL)
					free(header);
				if(sendbuf != NULL)
					free(sendbuf);
				//error?
				if(errflag)
					End_Thread(pthread_self());//end

			}//else end

		}
		/*********CASE:QUERYHIT END********/
		/*********CASE:PUSH********/
		else if(desc_header_rcv.payid == PUSH_ID){

		}
		/*********CASE:PUSH END********/
		/*********CASE:ERROR********/
		else{
			fprintf(stderr,"Unknown payid %d\n",desc_header_rcv.payid);
		}
		/*********CASE:ERROR END********/




	}//while(1) End

	return NULL;

}

void *
Th_Httpfunc(void *arg)
{
	//soon die!!
	if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) != 0){
		fprintf(stderr,"pthread_setcanceltype() error!\n");
	}

	//const
	const int BUFFER_SIZE = 1024;
	const int ARGV_SIZE = 64;

	int i;//for general
	int errflag = 0;
	int fd = *(int *)arg;
	int real_rcvsizeh,real_rcvsized;
	int real_sendsize;
	char header_buf_rcv[BUFFER_SIZE];
	char header_buf_send[BUFFER_SIZE];
	char *file_contents_rcv;
	char *file_contents_send;
	//int type = TYPE_HTTP;
	//for analyze
	int argc;
	char *argv[ARGV_SIZE];
	char *cmd_line = NULL;
	//for get cmd
	int len;
	char *p;
	uint32_t index;
	char *filename;
	Result_set result_set;
	char namebuf[MAX_FILENAME_SIZE];
	Status_Line status_line;
	//for http cmd
	int content_length;
	char download_filename[MAX_FILENAME_SIZE];



	memset(header_buf_rcv,0,sizeof(header_buf_rcv));
	/***************************COMMON RECV HEADER***********************/
	if((real_rcvsizeh = Http_Recv_Header(fd,header_buf_rcv,sizeof(header_buf_rcv))) <= 0){//recv header
		if(real_rcvsizeh == 0)
			fprintf(stderr,"Peer Shutdown Successfully\n");
		else
			fprintf(stderr,"Http_Recv_Header() error!\n");
		End_Thread(pthread_self());//end
	}
	/***************************COMMON RECV HEADER END************************/


	/************************Print Head********************************/
	//lock
	pthread_mutex_lock(&key_print);
	printf("\n---------------------------------------------------------\n");
	printf("HTTP connection %d recv header %d bytes \n",fd,real_rcvsizeh);
	printf("-----------------------------------------------------------\n");
	for(i=0; i<real_rcvsizeh; i++){
		printf("%c",header_buf_rcv[i]);
	}
	printf("\n");
	printf("-----------------------------------------------------------\n");
	//unlock
	pthread_mutex_unlock(&key_print);
	/************************Print Head********************************/

	//analyze cmdline
	if((cmd_line = Get_Line(header_buf_rcv,1,real_rcvsizeh)) == NULL){
		fprintf(stderr,"Get_Line() return NULL\n");
		End_Thread(pthread_self());//end
	}
	getargs(cmd_line,&argc,argv);
	if(argc <= 0){
		fprintf(stderr,"argc is 0\n");
		free(cmd_line);
		End_Thread(pthread_self());//end
	}


	//cmd
	/*************GET**************/
	if(strcmp(argv[0],GET) == 0){//File Send
		/* format check */
		if(argc != 3){//GET /get/2468/foo.mp3/ HTTP/1.0
			fprintf(stderr,"Client Bad Request Format , argc is not 3\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		len = strlen(argv[1]);
		if(Rm_Slash(argv[1],len,&index,&filename) < 0){
			fprintf(stderr,"Client Bad Request Format , Rm_Slash() error!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		//printf("index:%d file:%s\n",(int)index,filename);
		len = strlen(argv[2]);
		if((p = Search_Char(argv[2],'\r',len)) == NULL){
			fprintf(stderr,"Client Bad Request Format , Search_Char() error!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		*p = '\0';
		if(strcmp(argv[2],HTTP_1_0) != 0){
			fprintf(stderr,"Client Bad Request Format , Request Not HTTP/1.0!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		/* format check end */

		result_set = Get_Locallist_Result(index,filename);
		if(result_set.file_size == 0){
			fprintf(stderr,"Not Exist Such File index:%d name:%s\n",(int)index,filename);
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		snprintf(namebuf,sizeof(namebuf),"%s%s",Get_Share_Dir(),result_set.file_name);
		//printf("filename:%s\n",namebuf);
		//check file
		if(Check_File(namebuf) < 0){//Not Open File
			fprintf(stderr,"Check_File() Error!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		//Make Header
		status_line.version = "HTTP/1.0";
		status_line.code_and_reason = OK;
		Make_Status_Line(status_line,header_buf_send);
		Make_Object_Header(result_set.file_size,header_buf_send);
		Make_End(header_buf_send);
		//Make Contents
		if((file_contents_send= Get_File_Contents(namebuf,result_set.file_size)) == NULL){//error not memory
			fprintf(stderr,"Get_File_Contents() error!!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}

		//send header
		//lock (for send)
		pthread_mutex_lock(&key_thtbl);
		if((real_sendsize = Gnutella_Send(fd,header_buf_send,strlen(header_buf_send))) < 0){//send
			fprintf(stderr,"Gnutella_Send() error!\n");
			errflag = 1;
		}
		//unlock
		pthread_mutex_unlock(&key_thtbl);

		//send contents(file)
		//lock (for send)
		pthread_mutex_lock(&key_thtbl);
		if((real_sendsize = Gnutella_Send(fd,file_contents_send,result_set.file_size)) < 0){//send
			fprintf(stderr,"Gnutella_Send() error!\n");
			errflag = 1;
		}
		//unlock
		pthread_mutex_unlock(&key_thtbl);

		free(file_contents_send);
		if(errflag){
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		printf("/***Uploads Success!!***/ filename:%s\n",result_set.file_name);

	}
	/*************GET END**************/
	/************HTTP******************/
	else if(strcmp(argv[0],"HTTP/1.0") == 0 || strcmp(argv[0],"HTTP") == 0){//File Recv(download)

		if((content_length = Get_Content_Length(header_buf_rcv,real_rcvsizeh)) < 0){
			fprintf(stderr,"Content-Length Field is Nothing!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}
		//printf("length = %d bytes!!\n",content_length);

		if((file_contents_rcv = malloc(content_length)) == NULL){
			fprintf(stderr,"malloc() error!\n");
			free(cmd_line);
			End_Thread(pthread_self());//end
		}

		//recv
		if((real_rcvsized = Gnutella_Recv_Data(fd,file_contents_rcv,content_length)) <= 0){//recv file
			if(real_rcvsized == 0)
				fprintf(stderr,"Peer Shutdown Successfully\n");
			else
				fprintf(stderr,"Gnutella_Recv_Data() error!\n");
			End_Thread(pthread_self());//end
		}

		snprintf(download_filename,sizeof(download_filename),
				"%sGnutella_Download_%d",Get_Share_Dir(), content_length);
		//save downloads file
		if(Save_File(download_filename,file_contents_rcv,content_length) < 0){
			fprintf(stderr,"Save_File() error!\n");
			free(cmd_line);
			free(file_contents_rcv);
			End_Thread(pthread_self());//end
		}
		printf("/***Downloads Success!!***/ filename:%s\n",download_filename);

		free(file_contents_rcv);
	}
	/************HTTP END******************/
	/************GIV******************/
	else if(strcmp(argv[0],"GIV") == 0){

	}
	/************GIV END******************/
	else{
		fprintf(stderr,"Unknown Cmdline %s\n",argv[0]);
		End_Thread(pthread_self());//end
	}

	//free
	if(cmd_line != NULL)
		free(cmd_line);
	//End HTTP Thread!!
	End_Thread(pthread_self());//end

	return NULL;

}
