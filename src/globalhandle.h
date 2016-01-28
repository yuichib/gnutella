/*
 * globalhandle.h
 *
 *  Created on: 2009/04/28
 *      Author: yuichi
 */

void Init_Thtbl(void);
void Init_Ping_List(void);
void Init_Query_List(void);


void Init_Local_Sharelist(char *dir);
void Init_Net_Sharelist(void);
void Print_Local_Sharelist(void);
void Print_Net_Sharelist(void);
Result_set Get_Locallist_Result(uint32_t index,char *filename);
Result_setplus Get_Netlist_Resultplus(int id);

//int Get_New_Thindex(void);
int Create_Thread(int fd,int type);
int Kill_Thread(int fd);//(for foreign)
void End_Thread(pthread_t tid);//tid must get from pthread_self(for suicide)

void Register_Ping_List(uint8_t *guid,int fd);
int Check_Ping_List(uint8_t *guid);//rerurn -1(nothing) or 0(fromme) or fd(came from connection)
void Register_Query_List(uint8_t *guid,int fd);
int Check_Query_List(uint8_t *guid);//rerurn -1(nothing) or 0(fromme) or fd(came from connection)


void Register_Net_Sharelist(Queryhit_desc queryhit);

in_addr_t Get_Myip(void);
in_port_t Get_Myportg(void);
in_port_t Get_Myporth(void);
void Set_My_Servent_Id(uint8_t *servent_id);


void Search_Local_Sharelist(char *criteria,Result_set *result_set,int max,uint8_t *hits);

char *Get_Share_Dir(void);
int Get_Local_Share_Num(void);
int Get_Sharefile_Num(void);
int Get_Sharebyte_Num(void);
uint16_t Get_Myspeed(void);
