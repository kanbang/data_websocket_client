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

	CRITICAL_SECTION m_cs_subscribe;          // 注册锁      

	CRITICAL_SECTION m_cs_update_depth;        // 深度更新锁 

	WebSocketClient m_ws;
	reconn_setting_t m_reconn;
	http_headers m_headers;

	std::string m_wsurl;
	std::string m_wsid;

	long long m_onmessage_count;
	long long m_onmessage_ts;
	long long m_pingpong_ts;
	long long m_recvcount_depth;
	long long m_recvcount_trade;

	int m_connect_status;
	int m_startping;

	//      标准合约名
	std::map<std::string, STR_MYSUBSCRIBE_MARKET> m_map_subinfo;

	std::map<std::string, std::string> m_map_cnl2std;
	std::map<std::string, std::string> m_map_std2cnl;


	std::map<std::string,int> m_map_subinfo_depth;
	int m_subinfo_depth_level;
	int m_subinfo_depth_type;  // 数据类型  1 是数据全推   2 数据增量

	std::map<std::string,int> m_map_subinfo_trade;

	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Asc>> m_map_depthfull_ask;
	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Des>> m_map_depthfull_bid;

	virtual int m_thread_ping();

	virtual void on_open();
	virtual void on_close();
	virtual void on_message(const std::string& msg);

	virtual int mf_subscribe_single(const char* stdinstid, int datatype);
	virtual int mf_subscribe_all();
	virtual int mf_subscribe_depth_limit_all();
	virtual int mf_subscribe_depth_full_all();
	virtual int mf_unsubscribe_all_depth_limit();
	virtual int mf_unsubscribe_all_depth_full();

	virtual int mf_add_subinfo(const char* stdinstid, int datatype);

	virtual void open();

	virtual void mf_clear_depthfull();

	std::string m_logfile;

	std::string m_resturl_base;

	HttpClient* m_http_client;


};


#endif MYWEBSOCKETMARKET_H
