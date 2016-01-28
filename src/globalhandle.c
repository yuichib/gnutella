/*
 * globalhandle.c
 *
 *  Created on: 2009/04/28
 *      Author: yuichi
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>//typedef
#include <arpa/inet.h>//inet_aton
#include <unistd.h>//close
#include <sys/stat.h>//stat
#include "protocol.h"
#include "mythread.h"
#include "mystr.h"
#include "list.h"

#include "globalhandle.h"
#include "globalvar.h"//last



//key
pthread_mutex_t key_thtbl = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t key_system = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t key_print = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t key_pinglist = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t key_querylist = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t key_netlist = PTHREAD_MUTEX_INITIALIZER;



void
Init_Local_Sharelist(char *dir)
{

	int i = 0;
	const char *tmp = "local_shrelist.txt";
	char str[128];
	char path[128];
	char buf[128];
	char *p;
	FILE *fp;
	struct stat inode;

	snprintf(str,sizeof(str),"ls -1 %s > %s",dir, tmp);
	if(system(str) < 0){
		fprintf(stderr,"system() error!\n");
		exit(-1);
	}


	if((fp = fopen(tmp,"r")) == NULL){
		fprintf(stderr,"Can't Open File %s\n",tmp);
		exit(-1);
	}


	local_share_num = 0;
	while(fgets(buf,sizeof(buf),fp) != NULL){
		if(i >= MAX_LOCALLIST_NUM){
			fprintf(stderr,"Share File Too Many ! Please under %d ko\n",MAX_LOCALLIST_NUM);
			exit(-1);
		}
		if((p = Search_Char(buf,'\n',sizeof(buf))) == NULL){
			fprintf(stderr,"Search_Char() error!\n");
			exit(-1);
		}
		*p = '\0';
		if(strlen(buf) + 1 > MAX_FILENAME_SIZE){
			fprintf(stderr,"Too Long File Name %s\n",buf);
			exit(-1);
		}
		sscanf(buf,"%s",local_sharelist[i].file_name);
		local_sharelist[i].file_index = i;
		snprintf(path,sizeof(path),"%s%s",dir,local_sharelist[i].file_name);
		if(stat(path,&inode) < 0){
			printf("aa\n");
			perror("stat");
			exit(-1);
		}
		local_sharelist[i].file_size = (uint32_t)inode.st_size;
		i++;

	}
	local_share_num = i;
	fclose(fp);
	snprintf(str,sizeof(str),"rm -f %s",tmp);
	if(system(str) < 0){
		fprintf(stderr,"system() error!\n");
		exit(-1);
	}

}

void
Init_Net_Sharelist(void)
{

	//lock
	pthread_mutex_lock(&key_netlist);

	memset(net_sharelist,0,sizeof(Result_setplus)*MAX_NETLIST_NUM);

	//unlocak
	pthread_mutex_unlock(&key_netlist);

}


void
Print_Local_Sharelist(void)
{
	int i;
	for(i=0; i<Get_Local_Share_Num(); i++){
		printf("      - %d %d %s\n",
				(int)local_sharelist[i].file_index,(int)local_sharelist[i].file_size,local_sharelist[i].file_name);
	}

}

void
Print_Net_Sharelist(void)
{
	int i;
	struct in_addr tmp;


	//lock
	pthread_mutex_lock(&key_netlist);
	printf("/*********************Net Share List****************************/\n");
	printf("getid:    ip:        port:  speed:   index:  size:  filename:\n");
	for(i=0; i<MAX_NETLIST_NUM; i++){
		if(net_sharelist[i].result_set.file_size != 0){//exist
			tmp.s_addr = net_sharelist[i].ip;
			printf("%d:   %s   %d  %d       %d    %d    %s\n",
					i,
					inet_ntoa(tmp),net_sharelist[i].port,(int)net_sharelist[i].speed,
					(int)net_sharelist[i].result_set.file_index,(int)net_sharelist[i].result_set.file_size,
					net_sharelist[i].result_set.file_name);
		}
	}
	printf("/*********************Net Share List End**************************/\n");
	//unlocak
	pthread_mutex_unlock(&key_netlist);

}

Result_setplus
Get_Netlist_Resultplus(int id)
{
	int i;
	Result_setplus ret;
	//lock
	pthread_mutex_lock(&key_netlist);
	for(i=0; i<MAX_NETLIST_NUM; i++){
		if( (i == id) && (net_sharelist[i].result_set.file_size > 0) ){
			ret = net_sharelist[i];
			break;
		}
	}
	//unlocak
	pthread_mutex_unlock(&key_netlist);

	if(i == MAX_NETLIST_NUM){//Not Found
		memset(&ret,0,sizeof(ret));
	}

	return ret;

}


Result_set
Get_Locallist_Result(uint32_t index,char *filename)
{
	int i;
	Result_set ret;

	for(i=0; i<Get_Local_Share_Num(); i++){
		if( (local_sharelist[i].file_index == index) && (strcmp(local_sharelist[i].file_name,filename) == 0) ){
			ret = local_sharelist[i];
			break;
		}
	}

	if(i == Get_Local_Share_Num()){//Not Found
		ret.file_size = 0;
		ret.file_index = 0;
		memset(ret.file_name,0,sizeof(ret.file_name));
	}

	return ret;

}


int
Get_Local_Share_Num()
{
	return local_share_num;
}

char *
Get_Share_Dir()
{
	return share_dir;
}

void
Search_Local_Sharelist(char *criteria,Result_set *result_set,int max,uint8_t *hits)
{

	int i;

	*hits = 0;
	for(i=0; i<Get_Local_Share_Num(); i++){
		if(strstr(local_sharelist[i].file_name,criteria) != NULL){//found!!
			if(*hits < max){//max register
				result_set[*hits].file_index = local_sharelist[i].file_index;
				result_set[*hits].file_size = local_sharelist[i].file_size;
				strncpy(result_set[*hits].file_name,local_sharelist[i].file_name,sizeof(result_set[*hits].file_name));
				(*hits)++;
			}
		}
	}

}





int
Get_Sharefile_Num()
{
	/*
	int num;
	FILE *fp;
	const char *filename = "sharefilenum.txt";
	int errflag = 0;
	char str[64];
	snprintf(str,sizeof(str),"ls %s | wc -l > %s",Get_Share_Dir(),filename);
	//lock
	pthread_mutex_lock(&key_system);
	if(system(str) < 0){
		fprintf(stderr,"system() error!\n");
		errflag = 1;
	}
	else{
		if((fp = fopen(filename,"r")) == NULL){
			fprintf(stderr,"Can't Open File %s!\n",filename);
			errflag = 1;
		}
		else{
			fscanf(fp,"%d",&num);
			fclose(fp);
		}
		snprintf(str,sizeof(str),"rm -f %s",filename);
		if(system(str) < 0){
			fprintf(stderr,"system() error!\n");
			errflag = 1;
		}
	}
	//unlock
	pthread_mutex_unlock(&key_system);

	if(errflag == 0)
		return num;
	else
		return -1;
	 */
	return Get_Local_Share_Num();

}


int
Get_Sharebyte_Num()
{
	/*
	int num;
	FILE *fp;
	const char *filename = "sharebytenum.txt";
	int errflag = 0;
	char str[64];
	snprintf(str,sizeof(str),"du %s -S -s -k > %s",Get_Share_Dir(),filename);
	//lock
	pthread_mutex_lock(&key_system);
	if(system(str) < 0){
		fprintf(stderr,"system() error!\n");
		errflag = 1;
	}
	else{
		if((fp = fopen(filename,"r")) == NULL){
			fprintf(stderr,"Can't Open File %s!\n",filename);
			errflag = 1;
		}
		else{
			fscanf(fp,"%d",&num);
			fclose(fp);
		}
		snprintf(str,sizeof(str),"rm -f %s",filename);
		if(system(str) < 0){
			fprintf(stderr,"system() error!\n");
			errflag = 1;
		}
	}
	//unlock
	pthread_mutex_unlock(&key_system);

	if(errflag == 0)
		return num;
	else
		return -1;
	 */
	int i,sum = 0;
	const int kbyte = 1024;
	for(i=0; i<Get_Sharefile_Num(); i++){
		sum += local_sharelist[i].file_size;//bytes
	}
	sum /= kbyte;
	return sum;//kbytes

}




/*
void
Test()
{
	static int first = 1;
	int flag=0;
	if(first){
		thtbl[5].fd = 10;
		first=0;
		flag=1;
	}
	int i,tmp;
	for(i=0;i<5;i++){
		pthread_mutex_lock(&key_thtbl);
		tmp = thtbl[5].fd;
		tmp--;
		if(flag)
			sleep(2);
		thtbl[5].fd = tmp;
		pthread_mutex_unlock(&key_thtbl);
	}
	printf("%d\n",thtbl[5].fd);
}
 */

void
Init_Thtbl()
{
	int i;

	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){
		thtbl[i].tid = (pthread_t)TH_EMPTY;
		thtbl[i].fd = FD_EMPTY;
		thtbl[i].type = TYPE_EMPTY;
	}
	pthread_mutex_unlock(&key_thtbl);

}


void
Init_Ping_List()
{
	int i;
	pthread_mutex_lock(&key_pinglist);
	for(i=0; i<MAX_LIST_NUM; i++){
		memset(ping_list[i].guid,0,GUID_SIZE);
		ping_list[i].fd = NOTSEEN;
	}
	pthread_mutex_unlock(&key_pinglist);

}


void
Init_Query_List()
{
	int i;
	pthread_mutex_lock(&key_querylist);
	for(i=0; i<MAX_LIST_NUM; i++){
		memset(query_list[i].guid,0,GUID_SIZE);
		query_list[i].fd = NOTSEEN;
	}
	pthread_mutex_unlock(&key_querylist);

}


void
Register_Ping_List(uint8_t *guid,int fd)
{
	static int counter = 0;

	//lock
	pthread_mutex_lock(&key_pinglist);
	memcpy(ping_list[counter % MAX_LIST_NUM].guid,guid,GUID_SIZE);
	ping_list[counter % MAX_LIST_NUM].fd = fd;
	counter++;
	//unlock
	pthread_mutex_unlock(&key_pinglist);

}

void
Register_Query_List(uint8_t *guid,int fd)
{
	static int counter = 0;

	//lock
	pthread_mutex_lock(&key_querylist);
	memcpy(query_list[counter % MAX_LIST_NUM].guid,guid,GUID_SIZE);
	query_list[counter % MAX_LIST_NUM].fd = fd;
	counter++;
	//unlock
	pthread_mutex_unlock(&key_querylist);

}

//register_net_sharelist
void
Register_Net_Sharelist(Queryhit_desc queryhit){

	static int counter = 0;
	int i;
	int k;

	//lock
	pthread_mutex_lock(&key_netlist);
	for(i=0; i<queryhit.hits; i++){
		k = counter % MAX_NETLIST_NUM;

		net_sharelist[k].ip = queryhit.ip;
		net_sharelist[k].port = queryhit.port;
		net_sharelist[k].speed = queryhit.speed;
		net_sharelist[k].result_set.file_index = queryhit.result_set[i].file_index;
		net_sharelist[k].result_set.file_size = queryhit.result_set[i].file_size;
		strncpy(net_sharelist[k].result_set.file_name,queryhit.result_set[i].file_name,
				sizeof(net_sharelist[k].result_set.file_name));
		memcpy(net_sharelist[k].servent_id,queryhit.servent_id,
				SERVENT_ID_SIZE);

		counter++;
	}
	//unlock
	pthread_mutex_unlock(&key_netlist);


}


int
Check_Ping_List(uint8_t *guid)//rerurn -1(nothing) or 0(fromme) or fd(came from connection)
{
	int i;
	int ret;

	//lock
	pthread_mutex_lock(&key_pinglist);
	for(i=0; i<MAX_LIST_NUM; i++){
		if(memcmp(ping_list[i].guid,guid,GUID_SIZE) == 0){//same
			if(ping_list[i].fd == FROMME){//from me
				ret = FROMME;
			}
			else{//from other connection
				ret = ping_list[i].fd;
			}
			break;
		}
	}
	if(i == MAX_LIST_NUM)
		ret = NOTSEEN;
	//unlock
	pthread_mutex_unlock(&key_pinglist);

	return ret;

}


int
Check_Query_List(uint8_t *guid)//rerurn -1(nothing) or 0(fromme) or fd(came from connection)
{
	int i;
	int ret;

	//lock
	pthread_mutex_lock(&key_querylist);
	for(i=0; i<MAX_LIST_NUM; i++){
		if(memcmp(query_list[i].guid,guid,GUID_SIZE) == 0){//same
			if(query_list[i].fd == FROMME){//from me
				ret = FROMME;
			}
			else{//from other connection
				ret = query_list[i].fd;
			}
			break;
		}
	}
	if(i == MAX_LIST_NUM)
		ret = NOTSEEN;
	//unlock
	pthread_mutex_unlock(&key_querylist);

	return ret;

}


/* create thread */
int
Create_Thread(int fd,int type)
{
	if(type != TYPE_GNU && type != TYPE_HTTP){
		fprintf(stderr,"Unknown Thread Type!\n");
		return -1;
	}

	int i;
	int flag = 0;

	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){
		if(thtbl[i].tid == (pthread_t)TH_EMPTY)
			break;//must not return!! key keep locked!!
	}
	if(i == MAX_THREAD_NUM)//not empty
		flag = 1;
	//register and create
	if(flag == 0){
		thtbl[i].fd = fd;
		thtbl[i].type = type;
		if(type == TYPE_GNU){
			pthread_create(&thtbl[i].tid,NULL,Th_Gnufunc,(void *)&thtbl[i].fd);
		}
		else if(type == TYPE_HTTP){
			pthread_create(&thtbl[i].tid,NULL,Th_Httpfunc,(void *)&thtbl[i].fd);
		}
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);


	if(flag == 0){
		return i;//thtbl_inddex
	}
	else
		return -1;

}

/* end thread */
int
Kill_Thread(int fd)//(for foreign)
{

	if(fd == FD_EMPTY)
		return -1;

	int i;
	pthread_t tid;


	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){
		if(fd == thtbl[i].fd){
			tid = thtbl[i].tid;
			thtbl[i].tid = (pthread_t)TH_EMPTY;
			thtbl[i].fd = FD_EMPTY;
			thtbl[i].type = TYPE_EMPTY;
			break;
		}
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	if(i == MAX_THREAD_NUM){//no such connection fd
		fprintf(stderr,"Not Exist Such Connection %d\n",fd);
		return -1;
	}

	if(pthread_detach(tid) != 0){//free thread memory
		fprintf(stderr,"pthread_datach() error!\n");
	}

	if(pthread_cancel(tid) != 0){//kill thread
		fprintf(stderr,"pthread_cancel() error!\n");
		return -1;
	}
	close(fd);//close connection last!!

	return 1;

}

/* end thread */
void
End_Thread(pthread_t tid)//tid must get from pthread_self(for suicide)
{
	int i;

	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){//clear thtbl
		if(pthread_equal(thtbl[i].tid,tid) != 0){//when equal, return not 0
			close(thtbl[i].fd);//close connection
			thtbl[i].tid = (pthread_t)TH_EMPTY;
			thtbl[i].fd = FD_EMPTY;
			thtbl[i].type = TYPE_EMPTY;
			break;
		}
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	if(i == MAX_THREAD_NUM)
		fprintf(stderr,"Error End_Thread()!\n");

	if(pthread_detach(tid) != 0){//free thread memory
		fprintf(stderr,"pthread_datach() error!\n");
	}

	pthread_exit(NULL);//end suicide
}

in_addr_t
Get_Myip(void)
{
	return myip;

}

in_port_t
Get_Myportg(void)
{
	return myportg;

}

in_port_t
Get_Myporth(void)
{
	return myporth;

}

uint16_t
Get_Myspeed(void)
{
	return myspeed;
}


void
Set_My_Servent_Id(uint8_t *servent_id)
{

	in_addr_t ip = Get_Myip();
	in_port_t portg = Get_Myportg();
	in_port_t porth = Get_Myporth();
	uint8_t *p = servent_id;

	memcpy(p,&ip,sizeof(ip));
	p += sizeof(ip);
	memcpy(p,&portg,sizeof(portg));
	p += sizeof(portg);
	memcpy(p,&porth,sizeof(porth));
	p += sizeof(porth);
	memset(p,0,(SERVENT_ID_SIZE - sizeof(ip) - sizeof(portg) - sizeof(porth)));

}

/*
int Get_New_Thindex(void)
{
	int i;

	//lock
	pthread_mutex_lock(&key_thtbl);
	for(i=0; i<MAX_THREAD_NUM; i++){
		if(thtbl[i].tid == TH_EMPTY)
			break;//must not return!! key keep locked!!
	}
	//unlock
	pthread_mutex_unlock(&key_thtbl);

	if(i == MAX_THREAD_NUM)//not empty
		return -1;

	return i;
}
 */
