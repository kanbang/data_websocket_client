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

	int Disconnect();                            //�ж�socket����

	int ReConnect();                             //socket��channel������

	int mf_Add_MqMsg(STR_RABBITMQ* newmsg);     //socket��channel������

	STR_RABBITMQ* mf_Get_MqMsg();               //socket��channel������

	int mf_Run_Publish_RabbitmqMsg();

	int mf_send(const char* msg);

private:
	string                      m_strHostname;      // amqp����
	int                         m_iPort;            // amqp�˿�
	string                      m_connstr;          // �����ַ���
	string					    m_filter;           // �����ַ���
	int					        m_keepalive;        // �Ƿ�������������
	int                         m_keepidle;         // ����� ���
	zmq::context_t* m_context;
	zmq::socket_t* m_socket;
};


#endif