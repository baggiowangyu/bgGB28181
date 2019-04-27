// bgGB28181.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <winsock.h>
#endif

#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_port.h>

#include <eXosip2/eXosip.h>
#include <eXosip2/eX_setup.h>
#include <eXosip2/eX_register.h>
#include <eXosip2/eX_options.h>
#include <eXosip2/eX_message.h>



//////////////////////////////////////////////////////////////////////////
//
// 这里是常量定义
//
//////////////////////////////////////////////////////////////////////////
#define LOCAL_IP		"192.168.231.1"
#define LOCAL_PORT		5090
#define LOCAL_PORT_INT	atoi(LOCAL_PORT)
#define LOCAL_GBCODE	"100110000201000000"

#define SERVER_IP		"192.168.231.1"
#define SERVER_PORT		5060
#define SERVER_PORT_INT	atoi(SERVER_PORT)
#define SERVER_GBCODE	""

DWORD WINAPI EventHandleThread(LPVOID lpParam);

int Register(eXosip_t *sip_contect);

int _tmain(int argc, _TCHAR* argv[])
{
	int errCode = 0;

	// 1. 初始化sip环境
	eXosip_t *sip_contect = eXosip_malloc();
	errCode = eXosip_init(sip_contect);
	if (errCode != 0)
	{
		printf("Initialize sip context failed! errCode : %d\n", errCode);
		return errCode;
	}

	// 2. 监听sip端口
	errCode = eXosip_listen_addr(sip_contect, IPPROTO_UDP, LOCAL_IP, LOCAL_PORT, AF_INET, 0);
	if (errCode != 0)
	{
		printf("Listen local port failed. ip : %s, port : %d\n", LOCAL_IP, LOCAL_PORT);
		eXosip_quit(sip_contect);
		return errCode;
	}

	// 3. 开启事件监听线程
	HANDLE hThread = CreateThread(NULL, 0, EventHandleThread, sip_contect, 0, NULL);
	if (hThread == NULL)
	{
		errCode = GetLastError();
		printf("Start event handle thread failed. errcode : %d\n", errCode);
		eXosip_quit(sip_contect);
		return errCode;
	}

	// 等0.5秒，让处理线程启动
	Sleep(500);

	// 这里开始可以都是事件触发了
	// 4. 发送注册
	errCode = Register();

	return 0;
}

DWORD WINAPI EventHandleThread(LPVOID lpParam)
{
	eXosip_t *sip_contect = (eXosip_t *)lpParam;
	eXosip_event *sip_event = nullptr;

	while (true)
	{
		// 等待50毫秒，看看有没有消息进来
		if (!(sip_event = eXosip_event_wait(sip_contect, 0, 50)))
			continue;

		// 上锁
		eXosip_lock(sip_contect);

		// 定义原子操作
		eXosip_automatic_action(sip_contect);

		// 解锁
		eXosip_unlock(sip_contect);

		printf("event.type >>> %d\n", sip_event->type);

		switch (sip_event->type)
		{
		case EXOSIP_CALL_ACK:
			break;
		case EXOSIP_CALL_ANSWERED:
			break;
		case EXOSIP_REGISTRATION_FAILURE:
			// 注册失败
			break;
		case EXOSIP_REGISTRATION_SUCCESS:
			// 注册成功
			printf("Register success !\n");
			break;
		default:
			break;
		}

		eXosip_event_free(sip_event);
	}

	return 0;
}

int Register(eXosip_t *sip_contect)
{
	char from_user[4096] = { 0 };
	char proxy[4096] = { 0 };
	char route[4096] = { 0 };

	sprintf(from_user, "sip:%s@%s", LOCAL_GBCODE, LOCAL_IP);
	sprintf(proxy, "sip:%s@%s", SERVER_GBCODE, SERVER_IP);
	sprintf(route, "<sip:%s:%d;lr>", SERVER_IP, 35060);		// 暂时不知道这个35060是干什么的

	// 清理鉴权信息
	eXosip_clear_authentication_info(sip_contect);

	osip_message_t *reg = nullptr;
	int reg_id = eXosip_register_build_initial_register(sip_contect, from_user, proxy, route, 3600, &reg);

	// 
}
