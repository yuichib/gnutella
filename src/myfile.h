/*
 * myfile.h
 *
 *  Created on: 2009/04/07
 *      Author: yuichi
 */

int Check_File(char *filename);//return 1 or -1
int Get_File_Size(char *filename);//return bytes or -1
char * Get_File_Contents(char *filename,int filesize);//caller must free
int Save_File(char *savefile,char *buf,int bufsize);
