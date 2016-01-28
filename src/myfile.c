/*
 * myfile.c
 *
 *  Created on: 2009/04/07
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


int
Check_File(char *filename)
{
	int fd;
	if((fd = open(filename,O_RDONLY)) < 0){
		return -1;
	}
	close(fd);
	return 1;

}

int
Get_File_Size(char *filename)
{
	int bytes = 0;
	int c;
	FILE *fp;

	if((fp = fopen(filename,"r")) == NULL){
		return -1;
	}
	while((c = fgetc(fp)) != EOF){
		bytes++;
	}
	fclose(fp);

	return bytes;

}

/* improved version*/
char *
Get_File_Contents(char *filename,int filesize)
{
	int c;
	int count = 0;
	FILE *fp;
	char *file_contents;
	char *ret;

	if((fp = fopen(filename,"r")) == NULL){//404
		return NULL;
	}
	if((file_contents = (char *)malloc(filesize)) == NULL){
		fclose(fp);
		return NULL;
	}
	ret = file_contents;
	while(((c = fgetc(fp)) != EOF) && (count < filesize)){
		*file_contents++ = (char)c;
		count++;
	}
	fclose(fp);

	return ret;

}




int
Save_File(char *savefile,char *buf,int bufsize)
{
	int fd;

	if((fd = open(savefile,O_WRONLY|O_TRUNC|O_CREAT,0644)) < 0){
		perror("open");
		return -1;
	}
	if(write(fd,buf,bufsize) < 0){
		perror("write");
		close(fd);
		return -1;
	}

	close(fd);
	return 1;

}
