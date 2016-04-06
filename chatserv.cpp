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


USER_LIST client_list;//成员列表
void do_login(MESSAGE msg,int sock,struct sockaddr_in *cliaddr);
void do_logout(MESSAGE msg,int sock,struct sockaddr_in *cliaddr);
void do_sendlist(int sock,struct sockaddr_in *cliaddr);




void chat_srv(int sock)
{
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int n;
	MESSAGE msg;

	while(1)
	{
		memset(&msg,0,sizeof(msg));
		clilen=sizeof(cliaddr);
		n=recvfrom(sock,&msg,sizeof(msg),0,(struct sockaddr *)&cliaddr,&clilen);
		if(n<0)
		{
			if(errno==EINTR)
			  continue;
			ERROR_EXIT("recvfrom");
		}
		int cmd=htonl(msg.cmd);
		switch(cmd)
		{
			case C2S_LOGIN:
				do_login(msg,sock,&cliaddr);
				break;
			case C2S_LOGOUT:
				do_logout(msg,sock,&cliaddr);
				break;
			case C2S_ONLINE_USER:
				do_sendlist(sock,&cliaddr);
				break;
			default:
				break;
		}
	}
}


//登录函数
void do_login(MESSAGE msg,int sock,struct sockaddr_in *cliaddr)
{
	
	USER_INFO user;
	strcpy(user.username,msg.body);
	user.ip=cliaddr->sin_addr.s_addr;
	user.port=cliaddr->sin_port;


	//查找用户：
	USER_LIST::iterator it;
	for(it=client_list.begin();it!=client_list.end();++it)
	{
		if(strcmp(it->username,msg.body)==0)
		  break;
	}

	if(it==client_list.end())
	{
		cout<<"has a user login:"<<msg.body<<"<->"<<inet_ntoa(cliaddr->sin_addr)<<":"<<ntohs(cliaddr->sin_port)<<endl;
		client_list.push_back(user);
		USER_LIST::iterator iter=client_list.begin();
		MESSAGE reply_msg;
		memset(&reply_msg,0,sizeof(reply_msg));
		reply_msg.cmd=htonl(S2C_LOGIN_OK);
		sendto(sock,&reply_msg,sizeof(reply_msg),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
		int count = htonl((int)client_list.size());
		sendto(sock,&count,sizeof(int),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
		cout<<"sending user list information to: "<<msg.body<<"<->"<<inet_ntoa(cliaddr->sin_addr)<<":"<<ntohs(cliaddr->sin_port)<<endl;
		for(it=client_list.begin();it!=client_list.end();++it)
		{
			sendto(sock,&(*it),sizeof(USER_INFO),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
		}
		//向其他客户通知有新用户登录
		for(it=client_list.begin();it!=client_list.end();++it)
		{
			if(strcmp(it->username,msg.body)==0)
			  continue;
			struct sockaddr_in peeraddr;
			memset(&peeraddr,0,sizeof(peeraddr));
			peeraddr.sin_family=AF_INET;
			peeraddr.sin_port=it->port;
			peeraddr.sin_addr.s_addr=it->ip;
			msg.cmd=htonl(S2C_SOMEONE_LOGIN);
			memcpy(msg.body,&user,sizeof(user));
			if(sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)&peeraddr,sizeof(peeraddr))<0)
			  ERROR_EXIT("sendto");
		}

	}
	else
	{
		cout<<"user "<<msg.body<<" has already logined\n"<<endl;
		MESSAGE reply_msg;
		memset(&reply_msg,0,sizeof(reply_msg));
		reply_msg.cmd=htonl(S2C_ALREADY_LOGINED);
		sendto(sock,&reply_msg,sizeof(reply_msg),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
	}
}

//有用户推出
void do_logout(MESSAGE msg,int sock,struct sockaddr_in *cliaddr)
{
	cout<<"has a user logout:"<<msg.body<<"<->"<<inet_ntoa(cliaddr->sin_addr)<<":"<<ntohs(cliaddr->sin_port)<<endl;
	USER_LIST::iterator it;
	for(it=client_list.begin();it!=client_list.end();++it)
	{
		if(strcmp(it->username,msg.body)==0)
		  break;
	}
	if(it!=client_list.end())
	  client_list.erase(it);
	for(it=client_list.begin();it!=client_list.end();++it)
	{
		//if(strcmp(it->username,msg.body)==0)
		//  continue;
		struct sockaddr_in peeraddr;
		memset(&peeraddr,0,sizeof(peeraddr));
		peeraddr.sin_family=AF_INET;
		peeraddr.sin_port=it->port;
		peeraddr.sin_addr.s_addr=it->ip;
		msg.cmd=htonl(S2C_SOMEONE_LOGOUT);
		if(sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)&peeraddr,sizeof(peeraddr))<0)
		  ERROR_EXIT("sendto");
	}
}


//查询用户列表
void do_sendlist(int sock,struct sockaddr_in *cliaddr)
{
	MESSAGE msg;
	msg.cmd=htonl(S2C_ONLINE_USER);
	//发送一个msg 命令过去
	sendto(sock,(const char *)&msg,sizeof(msg),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
	//发送在线客户数
	int count=htonl((int)client_list.size());
	sendto(sock,(const char*)&count,sizeof(int),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
	//发送在线用户列表
	for(USER_LIST::iterator it=client_list.begin();it!=client_list.end();++it)
	{
		sendto(sock,&*it,sizeof(USER_INFO),0,(struct sockaddr *)cliaddr,sizeof(*cliaddr));
	}
}



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
