/*
 * gnutella.h
 *
 *  Created on: 2009/04/24
 *      Author: yuichi
 */



typedef struct{
	int size;
	char *data;
}Info_payload;

/*********SEND,RECV*********************************/
int Gnutella_Create_Waitfd(struct sockaddr_in myskt);
int Gnutella_Connect(int fd);
int Gnutella_Accept(int fd);
int Gnutella_Send(int fd,char *buf,int msglen);//return real send size
int Gnutella_Recv_Header(int fd,char *buf);//23bytes get
int Gnutella_Recv_Data(int fd,char *buf,int paylen);//payload get

/*******set************************************/
void
Set_Desc_Header(Desc_header *desc_header,
		uint8_t *guid,uint8_t payid,uint8_t ttl,uint8_t hops,uint32_t paylen);
void
Set_Pong_Desc(Pong_desc *pong_desc,
		in_port_t port,in_addr_t ip,uint32_t file_num,uint32_t kbyte_num);
void
Set_Query_Desc(Query_desc *query_desc,
		uint16_t speed,char *criteria);

void
Set_Queryhit_Desc(Queryhit_desc *queryhit_desc,
		uint8_t hits,in_port_t port,in_addr_t ip,uint32_t speed,Result_set *result_set,uint8_t *servent_id);


/**********Make Payload (must free)*******************/
char *Make_Desc_Header(Desc_header desc_header);//(must free)
Info_payload Make_Ping(void);
Info_payload Make_Pong(Pong_desc pong_desc);
Info_payload Make_Query(Query_desc query_desc);
Info_payload Make_Queryhit(Queryhit_desc queryhit_desc);


/*************get**************************************/
void Get_Desc_Header(char *buf,Desc_header *desc_header);
void Get_Ping(void);
void Get_Pong(char *buf,Pong_desc *pong_desc);
void Get_Query(char *buf,Query_desc *query_desc);
int  Get_Queryhit(char *buf,int bufsize,Queryhit_desc *queryhit_desc,Result_set *result_set,int max);

