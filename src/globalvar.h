/*
 * globalvar.h
 *
 *  Created on: 2009/04/28
 *      Author: yuichi
 */


/*This File is include Only glovalhandle.c*/

//nothing include!!

extern Th thtbl[];
extern char *share_dir;//share directory
extern int local_share_num;
extern Result_set local_sharelist[];
extern Result_setplus net_sharelist[];
extern in_addr_t myip;//big
extern in_port_t myportg;//little
extern in_port_t myporth;//little
extern List ping_list[];
extern List query_list[];
extern uint16_t myspeed;

