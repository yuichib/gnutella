/*
 * mystr.c
 *
 *  Created on: 2009/04/06
 *      Author: yuichi
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "mystr.h"


//int isblank(int c);


static int
Get_1Line_Size(char *linebuf){

	int i=0;
	while(*linebuf != '\n'){
		i++;
		linebuf++;
	}
	i++;
	return i;//contain \n

}

char *Search_Char
(char *str,int search,int size){

	int i = 0;
	char *p = str;
	for( ; *p != '\0' && i<size; p++){
		if(*p == (char)search){
			return p;
		}
		i++;
	}
	return NULL;

}

char * Search_Mem
(char *buf,int search,int size){

	int i = 0;
	char *p = buf;
	for( ; i<size; p++){
		if(*p == (char)search){
			return p;
		}
		i++;
	}
	return NULL;

}

/* Improved version */
char *Get_Line
(char *buf,int line,int bufsize){

	int num=0;//mojisuu
	int linenum = 1;//1 origin
	char *pmal;
	int malsize;

	while(1){
		if(num >= bufsize)//end buffer
			return NULL;
		if(linenum == line)
			break;
		if(*buf == '\n')
			linenum++;
		buf++;
		num++;
	}
	bufsize -= num;//rest buf size
	if(Search_Char(buf,'\n',bufsize) == NULL)//exist \n ?
		return NULL;
	//store line
	malsize = Get_1Line_Size(buf);
	if( (pmal = (char *)malloc(malsize)) == NULL){
		fprintf(stderr,"malloc() Error\n");
		return NULL;
	}
	strncpy(pmal,buf,malsize);
	pmal[malsize-1] = '\0';
	return pmal;

}





void getargs
(char *cp, int *argc, char **argv){

	*argc = 0;
	argv[*argc] = NULL;
	loop:
	while (*cp && isblank(*cp)) //skip space and tab
		cp++;

	if (*cp == '\0'){
		argv[(*argc)] = NULL;
		return;
	}
	argv[(*argc)++] = cp;

	while (*cp && !isblank(*cp)) //search for end of word
		cp++;
	if(*cp == '\0'){
		argv[(*argc)] = NULL;
		return;
	}

	*cp++ = '\0';
	goto loop;

}


char *
Cat_Data(void *data1,int size1,void *data2,int size2)
{
	char *ret,*pmal;
	if((pmal = (char *)malloc(size1 + size2)) == NULL){
		return NULL;
	}
	ret = pmal;
	memcpy(pmal,data1,size1);
	pmal += size1;
	memcpy(pmal,data2,size2);

	return ret;
}

