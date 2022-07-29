#include "MyZmqPublish.h"
#include "MyStruct.h"
#include "MyLogger.h"

MyZmqPublish::MyZmqPublish()
{
	/*
	m_socket = new zmq::socket_t();
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PUB);
	memcpy(m_socket, &socket, sizeof(zmq::socket_t));*/

	m_context = new zmq::context_t(1);
	m_socket = new zmq::socket_t(*m_context, ZMQ_PUB);
}

MyZmqPublish::~MyZmqPublish()
{
	m_socket->unbind(m_connstr);
}




int MyZmqPublish::mf_SetCon(string strHostname, int iPort, const char * filter)
{
	m_strHostname = strHostname;      // amqp����
	m_iPort = iPort;                  // amqp�˿�
	m_filter = filter;                // �����ַ���

	m_connstr = "";
	m_connstr += "tcp://";
	m_connstr += strHostname + ":";
	m_connstr += std::to_string(iPort);

	return 0;
}

int MyZmqPublish::mf_SetCon(string cnstr, const char * filter)
{
	m_connstr = "";
	m_connstr += cnstr;
	m_filter = filter;                // �����ַ���

	return 0;
}



int MyZmqPublish::Connect(const string& strHostname, int iPort)
{
	m_connstr = "";
	m_connstr += "tcp://";
	m_connstr += strHostname + ":";
	m_connstr += std::to_string(iPort);
	mf_Connect();
	return 0;
}


int MyZmqPublish::Connect(const string& cnstr)
{
	m_connstr = cnstr;
	mf_Connect();
	return 0;
}

int MyZmqPublish::mf_Connect()
{
	// ����������
	int tcp_keep_alive = 1;
	zmq_setsockopt(*m_socket, ZMQ_TCP_KEEPALIVE, &tcp_keep_alive, sizeof(tcp_keep_alive));

	// �������ӿ���30s�����ͱ����
	int tcp_keep_idle = 30;
	zmq_setsockopt(*m_socket, ZMQ_TCP_KEEPALIVE_IDLE, &tcp_keep_idle, sizeof(tcp_keep_idle));

	m_socket->bind(m_connstr);   // �����ǰ󶨵�ַ
	//m_socket->connect(m_connstr);   // ע�� publishģʽ������connect��ʽ
	return 0;
}


int MyZmqPublish::Disconnect()                            //�ж�socket����
{
	m_socket->unbind(m_connstr);
	return 0;
}


int MyZmqPublish::ReConnect()                             //socket��channel������
{
	m_socket->bind(m_connstr);   // �����ǰ󶨵�ַ
	return 0;
}


int MyZmqPublish::mf_Add_MqMsg(STR_RABBITMQ * newmsg)     //socket��channel������
{
	EnterCriticalSection(&m_cs);

	m_rabbitmq_list.push_back(newmsg);

	LeaveCriticalSection(&m_cs);

	return 0;
}



STR_RABBITMQ * MyZmqPublish::mf_Get_MqMsg()               //socket��channel������
{
	STR_RABBITMQ * newmq = NULL;

	EnterCriticalSection(&m_cs);

	if (m_rabbitmq_list.empty() == false)
	{
		newmq = m_rabbitmq_list.front();
		m_rabbitmq_list.pop_front();
	}

	LeaveCriticalSection(&m_cs);

	return newmq;
}


int MyZmqPublish::mf_Run_Publish_RabbitmqMsg()
{
	m_run = 1;
	int count = 0;
	while (m_run)
	{
		try
		{
			string mymsg = "hellozmq";
			mymsg += std::to_string(count);
			count++;
			{
				size_t len = strlen(mymsg.c_str());
				zmq::message_t msg(len);
				memcpy(msg.data(), mymsg.c_str(), len);
				bool ok = m_socket->send(msg); // ��������
				if (!ok){
					std::cout << "send failed" << std::endl;
				}
				printf("%s\n", mymsg.c_str());
			}
			Sleep(1000);
		}
		catch (std::exception & e)
		{
			std::string logmsg;
			logmsg += "ZmqPublish mf_Run_Publish_ZmqMsg failed!";
			logmsg += (char *)e.what();
			gcp_mylogger->mf_Add_Log((char*)logmsg.c_str());
			Sleep(10000);
		}

	}
	return 0;
}


int MyZmqPublish::mf_send(const char * mymsg)
{
	//printf("%s\n", mymsg);
	size_t len = strlen(mymsg);
	zmq::message_t msg(len);
	memcpy(msg.data(), mymsg, len);
	bool ok = m_socket->send(msg); // ��������
	if (!ok) {
		std::cout << "send failed" << std::endl;
	}

	return 0;
}