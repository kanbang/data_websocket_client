#pragma once
#ifndef MYWEBSOCKETMARKET_BINANCE_F_H
#define MYWEBSOCKETMARKET_BINANCE_F_H

#include "MyWebSocketMarket.h"
#include <vector>
#include <map>



class MyWebSocketMarket_Binance_f:public MyWebSocketMarket
{

public:

	MyWebSocketMarket_Binance_f(std::string clientid);
	MyWebSocketMarket_Binance_f();
	~MyWebSocketMarket_Binance_f();


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


	int mf_subscribe_all();

	int mf_subscribe_depth_all();    // ���� �������

	int mf_subscribe_trade_all();    // ȡ������ �������

	int cf_sub_unsub_all(int suborunsub, int subchannel, int limitorfull, int level);  // ���ĺ���ʵ��


	int mf_parse_depth(Document & doc,long long receivets);        // �������
	int mf_parse_depth_limit(Document& doc, long long receivets);  // ������� ��������
	int mf_parse_depth_full(Document& doc, long long receivets);   // ������� ��������

	int mf_parse_trade(Document& doc, long long receivets);     // �����ɽ�����

	int cf_restreq_all_depthimage(int level);  // ���� ������Ϣ
	int cf_restparse_all_depthimage(const char * stdsymbol,Document& doc);  // ���� ������Ϣ

	int m_thread_ping();  // ping �߳�

	int m_subscribe_id;

	std::map<std::string, long long> c_depthfull_lastu;   // ����������ű��  У��ʹ��

	int c_needflash_depthfull;   // �Ƿ���Ҫ ˢ�� ��������

};



#endif MYWEBSOCKETMARKET_BINANCE_F_H