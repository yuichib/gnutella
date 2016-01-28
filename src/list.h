/*
 * list.h
 *
 *  Created on: 2009/04/30
 *      Author: yuichi
 */



//ping,query list
#define MAX_LIST_NUM 100
#define FROMME 0
#define NOTSEEN -1
#define DUMMY_GUID (uint8_t *)"AAAABBBBCCCCDDDD"


//local list
#define MAX_LOCALLIST_NUM 100
//share list
#define MAX_NETLIST_NUM 100

int Issue_Guid(uint8_t *guid);
