#pragma once
#ifndef MYWEBSOCKETMARKET_OKX_F_H
#define MYWEBSOCKETMARKET_OKX_F_H

#include "MyWebSocketMarket.h"
#include <vector>
#include <map>


class MyWebSocketMarket_Okx_f :public MyWebSocketMarket
{

public:

	MyWebSocketMarket_Okx_f(std::string clientid);
	MyWebSocketMarket_Okx_f();
	~MyWebSocketMarket_Okx_f();


	void on_open();
	void on_close();
	void on_message(const std::string& msg);

	std::map<int, long long> m_channeldata_count;

	/*
	std::string mf_cnlinstid2stdinstid(const char* cnlinstid);
	std::string mf_stdinstid2cnlinstid(const char* stdinstid);
	int mf_cnlinstid2stdinstid(const char* cnlinstid, char* stdinstidbuf);
	int mf_stdinstid2cnlinstid(const char* stdinstid, char* cnlinstidbuf);
	*/

	std::string mf_stdinstid2cnlinstid(const char* stdinstid);
	std::string mf_cnlinstid2stdinstid(const char* cnlinstid);

	

	int mf_subscribe_all();

	int mf_subscribe_depth_all();     // ���� �������
	int mf_unsubscribe_depth_all();   // ȡ������ �������

	int mf_subscribe_trade_all();     // ���� �ɽ�����
	int mf_unsubscribe_trade_all();   // ���� �ɽ�����


	int cf_sub_unsub_all(int suborunsub,int subchannel,int limitorfull,int level);  // ���ĺ���ʵ��


	int mf_parse_depth(Document& doc, long long receivets);         // �������
	int mf_parse_depth_limit(Document& doc, long long receivets);   // ������� ��������
	int mf_parse_depth_full(Document& doc, long long receivets);    // ������� ��������

	int mf_parse_trade(Document& doc, long long receivets);    // �����ɽ�����

	DWORD cf_getCRC(BYTE* ptr, DWORD Size);

	bool cf_check_depthfull(const char* stdinstid, UINT32 excrc);  // ��������˶�

	int m_thread_ping();  // ping�߳�

	int m_subscribe_id;

	int c_needflash_depthfull;     // �Ƿ���Ҫ ˢ�� ��������
};



#endif MYWEBSOCKETMARKET_OKX_F_H