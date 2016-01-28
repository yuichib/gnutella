/*
 * mystr.h
 *
 *  Created on: 2009/04/06
 *      Author: yuichi
 */


char * Search_Char(char *str,int search,int size);
char * Search_Mem(char *buf,int search,int size);
char *Get_Line(char *buf,int line,int bufsize);
void getargs(char *cp, int *argc, char **argv);
char *Cat_Data(void *data1,int size1,void *data2,int size2);//free
