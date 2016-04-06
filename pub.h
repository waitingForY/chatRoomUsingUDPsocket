#ifndef _PUB_H
#define _PUB_H
#include <list>
#include <algorithm>
using namespace std;

//c2s
#define C2S_LOGIN 1
#define C2S_LOGOUT 2
#define C2S_ONLINE_USER 3

#define MSG_LEN 512

//s2c
#define S2C_LOGIN_OK 1
#define S2C_ALREADY_LOGINED 2
#define S2C_SOMEONE_LOGIN 3
#define S2C_SOMEONE_LOGOUT 4
#define S2C_ONLINE_USER 5

//c2c
#define C2C_CHAT 6

typedef struct message
{
	int cmd;
	char body[MSG_LEN];
}MESSAGE;

typedef struct user_info
{
	char username[16];
	unsigned int ip;
	unsigned short port;
}USER_INFO;

typedef struct chat_msg
{
	char username[16];
	char msg[1024];
}CHAT_MSG;

typedef list<USER_INFO> USER_LIST;
#endif //_PUB_H
