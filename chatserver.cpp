#include "pub.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <iostream>
#define ERROR_EXIT(m) do{perror(m);exit(EXIT_FAILURE);}while(0)

int main(void)
{
	int sock;
	if((sock=socket(PF_INET,SOCK_DGRAM,0))<0)
	  ERROR_EXIT("socket");
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(5188);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	  ERROR_EXIT("bind");
	chat_srv(sock);
	return 0;
}
