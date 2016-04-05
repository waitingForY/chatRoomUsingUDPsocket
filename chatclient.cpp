#include "pub.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#define ERROR_EXIT(m) do{perror(m);exit(EXIT_FAILURE);}while(0)

int main(void)
{
	int sock;
	if((sock=socket(PF_INET,SOCK_DGRAM,0))<0)
	  ERROR_EXIT("socket");
	chat_cli(sock);
	return 0;
}
