/*
 * getip.c
 *
 *  Created on: 2009/04/30
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
//#include <linux/if.h> //delete Windows Mac, add linux
#include <net/if.h> //WIN,Linux,Mac OK!
#include <sys/ioctl.h>

#include "getip.h"




in_addr_t GetMyIpAddr(char *os) {

	int s = socket(AF_INET, SOCK_STREAM, 0);
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, os, IFNAMSIZ-1); //see ifconfig
	ioctl(s, SIOCGIFADDR, &ifr);
	close(s);

	struct sockaddr_in addr;
	memcpy( &addr, &ifr.ifr_ifru.ifru_addr, sizeof(struct sockaddr_in) );
	return addr.sin_addr.s_addr;

}
