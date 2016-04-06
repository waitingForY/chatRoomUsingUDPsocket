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


USER_LIST client_list;//成员列表
char username[16];//当前用户名
void do_someone_login(MESSAGE &msg);
void do_someone_logout(MESSAGE &msg);
void do_getlist(int sock);
void do_chat(MESSAGE &msg);

void parse_cmd(char *cmdline,int sock,struct sockaddr_in *servaddr);
bool sendmsgto(int sock,char *username,char *msg);
bool send_msg_to_server(int sock,char *msg,struct sockaddr_in *servaddr);

void showcmd()
{
	cout<<endl;
	cout<<"*****************************"<<endl;
	cout<<"命令如下:"<<endl;
	cout<<"1、ls"<<endl;
	cout<<"2、man"<<endl;
	cout<<"3、exit"<<endl;
	cout<<"4、list"<<endl;
	cout<<"5、clear"<<endl;
	cout<<"6、showinfo"<<endl;
	cout<<"7、chmod mode(mode:room/p2p)"<<endl;
	cout<<"*****************************"<<endl;
	cout<<endl;
}

/*
 *显示帮助信息
 */
void show_help_info()
{
	cout<<"1、ls展示功能命令！"<<endl;
	cout<<"2、man获取帮助文档！"<<endl;
	cout<<"3、exit退出聊天！"<<endl;
	cout<<"4、list查看用户列表！"<<endl;
	cout<<"5、clear清空聊天记录！"<<endl;
	cout<<"6、showinfo查看个人信息！"<<endl;
	cout<<"7、send username msg 向username发送msg，username必须存在（不能是自己）"<<endl;
	cout<<"8、send all msg 向所有人发送msg（all为关键字，用户名不能为all）"<<endl;
	cout<<"9、chmod mode选择聊天模式,mode为聊天模式（提供公聊room和私聊两种模式p2p）！"<<endl;

}
/*
 *私聊函数
 */
void do_chat(MESSAGE &msg)
{
	CHAT_MSG *chatmsg=(CHAT_MSG *)msg.body;
	cout<<chatmsg->username<<" 对你说: ["<<chatmsg->msg<<"]"<<endl;
}

void do_pubchat(MESSAGE &msg)
{
	CHAT_MSG *chatmsg=(CHAT_MSG *)msg.body;
	cout<<chatmsg->username<<" 说："<<" ["<<chatmsg->msg<<"]"<<endl;
}

/*
 *命令解析函数
 *
 */
void parse_cmd(char *cmdline,int sock,struct sockaddr_in *servaddr)
{
	char cmd[10]={0};
	char *p;
	p=strchr(cmdline,' ');
	if(p!=NULL)
	  *p='\0';
	strcpy(cmd,cmdline);
	
	if(strcmp(cmd,"exit")==0)
	{
		MESSAGE msg;
		memset(&msg,0,sizeof(msg));
		msg.cmd=htonl(C2S_LOGOUT);
		strcpy(msg.body,username);
		if(sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)servaddr,sizeof(*servaddr))<0)
		  ERROR_EXIT("sendto");
		cout<<"你已经成功退出聊天室！"<<endl;
		exit(EXIT_SUCCESS);
	}
	else if(strcmp(cmd,"send")==0)
	{
		char peername[16]={0};
		char msg[MSG_LEN]={0};
		/* send user msg */
		/*      p    p2  */
		while(*(++p)==' ');
		char *p2;
		p2=strchr(p,' ');
		if(p2==NULL)
		{
			cout<<"命令错误，请从新输入："<<endl;
			showcmd();
		}
		*p2='\0';
		strcpy(peername,p);
		while(*(++p2)==' ');
		strcpy(msg,p2);
		bool sendsucc;
		if(strcmp(peername,"all")==0)
		{
			if(!(sendsucc=send_msg_to_server(sock,msg,servaddr)))
			  cout<<"send false!"<<endl;
		}
		else
		{
			if(!(sendsucc=sendmsgto(sock,peername,msg)))
			  cout<<"send false!"<<endl;
		}
	}
	else if(strcmp(cmd,"list")==0)
	{
		MESSAGE msg;
		memset(&msg,0,sizeof(msg));
		msg.cmd=htonl(C2S_ONLINE_USER);
		if(sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)servaddr,sizeof(*servaddr))<0)
		  ERROR_EXIT("sendto");	
	}
	else if(strcmp(cmd,"showinfo")==0)
	{
		for(USER_LIST::iterator it=client_list.begin();it!=client_list.end();++it)
		  if(strcmp(it->username,username)==0)
		  {
			  in_addr tmp;
			  tmp.s_addr=it->ip;
			  cout<<it->username<<" ("<<inet_ntoa(tmp)<<":"<<ntohs(it->port)<<")"<<endl;

		  }
	}
	else if(strcmp(cmd,"man")==0)
	{
		cout<<"有如下命令可以使用："<<endl;
		show_help_info();
	}
	else if(strcmp(cmd,"clear")==0)
	{
		system("clear");
		//cout<<"该功能还在实现中，请耐心等候！"<<endl;
	}
	else if(strcmp(cmd,"chmod")==0)
	{
		char mode[16]={0};
		while(*(++p)==' ');
		char *p2;
		p2=p;
		//p2=strchr(p,' ');
		if(p2==NULL)
		{
			cout<<"命令错误，请从新输入："<<endl;
			showcmd();
		}
		//*p2='\0';
		strcpy(mode,p2);
		if(strcmp(mode,"room")==0)
		{
			system("clear");
			cout<<"现在你可以进行群聊了！"<<endl;
			cout<<"******************"<<endl;
			cout<<"send all msg"<<endl;
			cout<<"******************"<<endl;

		}
		else if(strcmp(mode,"p2p")==0)
		{
			system("clear");
			cout<<"现在你可以进行点对点通信了！"<<endl;
			cout<<"******************"<<endl;
			cout<<"send username msg"<<endl;
			cout<<"******************"<<endl;
		}

		else
		{
			cout<<"命令错误，请从新输入："<<endl;
			cout<<"***************************"<<endl;
			cout<<"chmod mode (mode:room/p2p)"<<endl;
			cout<<"***************************"<<endl;
		}
		//cout<<mode<<endl;
		//cout<<"该功能还在实现中，请耐心等候！"<<endl;

	}
	else if(strcmp(cmd,"ls")==0)
	{
		showcmd();
	}
	else
	{
		cout<<"命令错误，请从新输入："<<endl;
		showcmd();
	}
}



void do_someone_login(MESSAGE &msg)
{
	USER_INFO *user=(USER_INFO*)msg.body;
	in_addr tmp;
	tmp.s_addr=user->ip;
	cout<<user->username<<" ("<<inet_ntoa(tmp)<<":"<<ntohs(user->port)<<") 进入了聊天室！"<<endl;
	client_list.push_back(*user);
}


void do_someone_logout(MESSAGE &msg)
{
	USER_LIST::iterator it;
	for(it=client_list.begin();it!=client_list.end();++it)
	{
		if(strcmp(it->username,msg.body)==0)
		  break;
	}
	if(it!=client_list.end())
	  client_list.erase(it);
	cout<<msg.body<<" 退出了聊天室！"<<endl;
}

void do_getlist(int sock)
{
	int count;
	recvfrom(sock,&count,sizeof(int),0,NULL,NULL);
	cout<<endl;
	cout<<"目前共有 "<<ntohl(count)<<" 个人在此聊天室中！"<<endl;
	client_list.clear();
	int n=ntohl(count);
	for(int i=0;i<n;i++)
	{
		USER_INFO user;
		memset(&user,0,sizeof(USER_INFO));
		recvfrom(sock,&user,sizeof(USER_INFO),0,NULL,NULL);
		client_list.push_back(user);
		in_addr tmp;
		tmp.s_addr=user.ip;
		cout<<i+1<<" "<<user.username<<"("<<inet_ntoa(tmp)<<":"<<ntohs(user.port)<<")"<<endl;
	}
}
/*
 *
 *公聊函数，向服务器发送消息
 */
bool send_msg_to_server(int sock,char *msg,struct sockaddr_in *servaddr)
{
	MESSAGE message;
	memset(&message,0,sizeof(message));
	message.cmd=htonl(PUB_CHAT);
	CHAT_MSG chatmsg;
	strcpy(chatmsg.username,username);
	strcpy(chatmsg.msg,msg);
	memcpy(message.body,&chatmsg,sizeof(chatmsg));

	if(sendto(sock,&message,sizeof(message),0,(struct sockaddr *)servaddr,sizeof(*servaddr))<0)
	{
		return false;
		ERROR_EXIT("sendto");
	}
	return true;
}

/*
 *
 *私聊函数，可以进行p2p聊天
 */

bool sendmsgto(int sock,char *name,char *msg)
{
	if(strcmp(name,username)==0)
	{
		cout<<endl;
		cout<<"你不能对自己发消息！sb"<<endl;
		cout<<endl;
		return false;
	}
	USER_LIST::iterator it;
	for(it=client_list.begin();it!=client_list.end();++it)
	{
		if(strcmp(it->username,name)==0)
		  break;
	}
	if(it==client_list.end())
	{
		cout<<endl;
		cout<<"没有 "<<name<<" 这个用户，请先看聊天室有哪些人！"<<endl;
		cout<<endl;
		return false;
	}
	MESSAGE m;
	memset(&m,0,sizeof(m));
	m.cmd=htonl(C2C_CHAT);
	CHAT_MSG cm;
	strcpy(cm.username,username);
	strcpy(cm.msg,msg);
	memcpy(m.body,&cm,sizeof(cm));
	
	struct sockaddr_in peeraddr;
	memset(&peeraddr,0,sizeof(peeraddr));
	peeraddr.sin_family=AF_INET;
	peeraddr.sin_addr.s_addr=it->ip;
	peeraddr.sin_port=it->port;
	
	in_addr tmp;
	tmp.s_addr=it->ip;
	//cout<<"你对 "<<name<<" 说： ["<<msg<<"]"<<endl;
	sendto(sock,(const char *)&m,sizeof(m),0,(struct sockaddr *)&peeraddr,sizeof(peeraddr));
	return true;
}
void chat_cli(int sock)
{
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(5188);
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	MESSAGE msg;

	while(1)
	{
		memset(&username,0,sizeof(username));
		cout<<"请输入用户名(最好英文)：";
		fflush(stdout);
		cin>>username;

		memset(&msg,0,sizeof(msg));
		msg.cmd=htonl(C2S_LOGIN);
		strcpy(msg.body,username);

		sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
		memset(&msg,0,sizeof(msg));
		recvfrom(sock,&msg,sizeof(msg),0,NULL,NULL);
		int cmd=ntohl(msg.cmd);
		if(cmd==S2C_ALREADY_LOGINED)
		{
			cout<<endl;
			cout<<username<<" 这个用户名已存在，请重新输入！"<<endl;
			cout<<endl;
		}
		else if(cmd==S2C_LOGIN_OK)
		{
			cout<<endl;
			cout<<"恭喜你已经成功登录聊天室！"<<endl;
			cout<<"现在可以进行聊天了！"<<endl;
			cout<<endl;
			break;
		}
	}
	int count;
	recvfrom(sock,&count,sizeof(int),0,NULL,NULL);
	int n=ntohl(count);
	cout<<endl;
	cout<<"目前共有 "<<n<<" 个人在该聊天室中:"<<endl;
	cout<<endl;
	for(int i=0;i<n;i++)
	{
		USER_INFO user;
		recvfrom(sock,&user,sizeof(USER_INFO),0,NULL,NULL);
		client_list.push_back(user);
		in_addr tmp;
		tmp.s_addr=user.ip;
		cout<<i+1<<" "<<user.username<<" ("<<inet_ntoa(tmp)<<":"<<ntohs(user.port)<<")"<<endl;
	}
	cout<<endl;
	showcmd();
	cout<<endl;


//IO复用模型
	fd_set rset;
	FD_ZERO(&rset);
	int nready;
	while(1)
	{
		FD_SET(STDIN_FILENO,&rset);
		FD_SET(sock,&rset);
		nready=select(sock+1,&rset,NULL,NULL,NULL);
		if(nready==-1)
		  ERROR_EXIT("select");
		if(nready==0)
		  continue;
		if(FD_ISSET(sock,&rset))
		{
			peerlen=sizeof(peeraddr);
			memset(&msg,0,sizeof(msg));
			recvfrom(sock,&msg,sizeof(msg),0,(struct sockaddr *)&peeraddr,&peerlen);
			int cmd=ntohl(msg.cmd);
			switch(cmd)
			{
				case S2C_SOMEONE_LOGIN:
					do_someone_login(msg);
					break;
				case S2C_SOMEONE_LOGOUT:
					do_someone_logout(msg);
					break;
				case S2C_ONLINE_USER:
					do_getlist(sock);
					break;
				case C2C_CHAT:
					do_chat(msg);
					break;
				case PUB_CHAT:
					do_pubchat(msg);
					break;
				default:
					break;
			}
		}
		
		if(FD_ISSET(STDIN_FILENO,&rset))
		{
			char cmdline[1024]={0};
			if(fgets(cmdline,sizeof(cmdline),stdin)==NULL)
			  break;
			if(cmdline[0]=='\n')
			  continue;
			cmdline[strlen(cmdline)-1]='\0';
			parse_cmd(cmdline,sock,&servaddr);
		}
	}


}

int main(void)
{
	int sock;
	if((sock=socket(PF_INET,SOCK_DGRAM,0))<0)
	  ERROR_EXIT("socket");
	chat_cli(sock);
	return 0;
}
