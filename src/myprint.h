/*
 * myprint.h
 *
 *  Created on: 2009/04/25
 *      Author: yuichi
 */

void Print_Mem(char *ptr,int size);
void Print_Desc_Header(Desc_header desc_header);
void Print_Ping(void);
void Print_Pong(Pong_desc pong_desc);
void Print_Query(Query_desc query_desc);
void Print_Queryhit(Queryhit_desc queryhit_desc);


void Print_Direction(int direction);
