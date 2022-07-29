#pragma once
#ifndef MYWEBSOCKETMARKET_H
#define MYWEBSOCKETMARKET_H

#include "stdio.h"
#include "stdlib.h"
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "hv/WebSocketClient.h"
#include "hv/requests.h"

//#include "MyStruct.h"

using namespace rapidjson;

using namespace hv;


struct STR_MYDEPTHFULL
{
	long long updateid;
	long long updatets;
	double vol;
	char pricestr[50];
	char volstr[50];

	STR_MYDEPTHFULL()
	{
		updateid = 0;
		updatets = 0;
		vol = 0;
		strcpy(pricestr, "0");
		strcpy(volstr, "0");
	}
};




struct StrCompare_Asc {
	bool operator()(double f1, double f2) const
	{
		return f1 < f2;
	}
};

struct StrCompare_Des {
	bool operator()(double f1, double f2) const
	{
		return f1 > f2;
	}
};



class MyWebSocketMarket
{

public:

	MyWebSocketMarket(std::string clientid);
	MyWebSocketMarket();
	~MyWebSocketMarket();

	CRITICAL_SECTION m_cs_subscribe;          // ע����      

	CRITICAL_SECTION m_cs_update_depth;        // ��ȸ����� 

	WebSocketClient m_ws;
	reconn_setting_t m_reconn;
	http_headers m_headers;

	std::string m_wsurl;           // ws ��ַ
	std::string m_wsid;            // ws id

	
	long long m_onmessage_count;   // ��Ϣ��������
	long long m_onmessage_ts;      // ���һ����Ϣ����ʱ��
	long long m_pingpong_ts;       // ���һ��pingʱ��
	long long m_recvcount_depth;   // ���յ� ������� ����
	long long m_recvcount_trade;   // ���յ� �������� ����

	int m_connect_status;          // ����״̬
	int m_startping;               // ping�߳��Ƿ�����


	std::map<std::string, std::string> m_map_cnl2std;    // ��������Լ���� �� ��׼��Լ����
	std::map<std::string, std::string> m_map_std2cnl;    // ��׼��Լ���� �� ��������Լ����


	std::map<std::string,int> m_map_subinfo_depth;       // ���ĵ��������
	int m_subinfo_depth_level; // �������ĵ���
	int m_subinfo_depth_type;  // ��������  1 ������ȫ��   2 ��������

	std::map<std::string,int> m_map_subinfo_trade;       // ���ĵĽ�������

	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Asc>> m_map_depthfull_ask;   // ��������洢�� ����
	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Des>> m_map_depthfull_bid;   // ��������洢�� ���


	// ���º�����Ҫ������������ʵ��

	virtual int m_thread_ping();     // ping�߳�  

	virtual void on_open();          // ws �ص�  ���ӻص�
	virtual void on_close();         // ws �ص�  ���߻ص�
	virtual void on_message(const std::string& msg); // ws �ص�  ��Ϣ�ص�

	virtual int mf_subscribe_all();  // ������������

	virtual int mf_add_subinfo(const char* stdinstid, int datatype);   // ��Ӵ����ĵ�����

	virtual void open();    // ���� 

	virtual void mf_clear_depthfull();   // ���� ��������洢��

	std::string m_logfile;       // log λ��

	std::string m_resturl_base;  // http ����baseurl 
	
	HttpClient* m_http_client;   // http client


};


#endif MYWEBSOCKETMARKET_H
