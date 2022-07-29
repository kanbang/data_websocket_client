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

	std::string mf_stdinstid2cnlinstid(const char* stdinstid);

	int mf_subscribe_all();

	int mf_subscribe_depth_all();

	int mf_subscribe_trade_all();

	int cf_sub_unsub_all(int suborunsub, int subchannel, int limitorfull, int level);


	int mf_parse_depth(Document & doc,long long receivets);
	int mf_parse_depth_limit(Document& doc, long long receivets);
	int mf_parse_depth_full(Document& doc, long long receivets);

	int mf_parse_trade(Document& doc, long long receivets);

	int cf_restreq_all_depthimage(int level);
	int cf_restparse_all_depthimage(const char * stdsymbol,Document& doc);

	int m_subscribe_id;

	int m_thread_ping();

	std::map<std::string, long long> c_depthfull_lastu;

	int c_needflash_depthfull;

};



#endif MYWEBSOCKETMARKET_BINANCE_F_H