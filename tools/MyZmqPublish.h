#ifndef MYZMQPUBLISH_H_
#define MYZMQPUBLISH_H_

#include<stdlib.h>
#include<stdio.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <zmq.h>
//#include <zmq_utils.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <list>
#pragma comment(lib,"libzmq-v140-mt-4_3_4")

using namespace std;

struct STR_RABBITMQ;

class MyZmqPublish
{
public:
	MyZmqPublish();
	~MyZmqPublish();

	CRITICAL_SECTION m_cs;
	int m_run;
	std::list<STR_RABBITMQ*> m_rabbitmq_list;


	int mf_SetCon(string strHostname, int iPort, const char* filter = "");

	int mf_SetCon(string cnstr, const char* filter = "");

	int Connect(const string& strHostname, int iPort);

	int Connect(const string& cnstr);

	int mf_Connect();

	int Disconnect();                            //切断socket连接

	int ReConnect();                             //socket到channel的重连

	int mf_Add_MqMsg(STR_RABBITMQ* newmsg);     //socket到channel的重连

	STR_RABBITMQ* mf_Get_MqMsg();               //socket到channel的重连

	int mf_Run_Publish_RabbitmqMsg();

	int mf_send(const char* msg);

private:
	string                      m_strHostname;      // amqp主机
	int                         m_iPort;            // amqp端口
	string                      m_connstr;          // 链接字符串
	string					    m_filter;           // 过滤字符串
	int					        m_keepalive;        // 是否启动断线重连
	int                         m_keepidle;         // 保活包 间隔
	zmq::context_t* m_context;
	zmq::socket_t* m_socket;
};


#endif