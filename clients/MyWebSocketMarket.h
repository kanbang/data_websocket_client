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

	std::string m_wsurl;           // ws 地址
	std::string m_wsid;            // ws id

	
	long long m_onmessage_count;   // 消息接收数量
	long long m_onmessage_ts;      // 最后一次消息接收时间
	long long m_pingpong_ts;       // 最后一次ping时间
	long long m_recvcount_depth;   // 接收到 深度行情 数量
	long long m_recvcount_trade;   // 接收到 交易行情 数量

	int m_connect_status;          // 连接状态
	int m_startping;               // ping线程是否启动


	std::map<std::string, std::string> m_map_cnl2std;    // 交易所合约名称 到 标准合约名称
	std::map<std::string, std::string> m_map_std2cnl;    // 标准合约名称 到 交易所合约名称


	std::map<std::string,int> m_map_subinfo_depth;       // 订阅的深度行情
	int m_subinfo_depth_level; // 深度行情的档数
	int m_subinfo_depth_type;  // 数据类型  1 是数据全推   2 数据增量

	std::map<std::string,int> m_map_subinfo_trade;       // 订阅的交易行情

	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Asc>> m_map_depthfull_ask;   // 增量行情存储器 卖价
	std::map<std::string, std::map<double, STR_MYDEPTHFULL, StrCompare_Des>> m_map_depthfull_bid;   // 增量行情存储器 买价


	// 以下函数需要根据需求重载实现

	virtual int m_thread_ping();     // ping线程  

	virtual void on_open();          // ws 回调  连接回调
	virtual void on_close();         // ws 回调  断线回调
	virtual void on_message(const std::string& msg); // ws 回调  消息回调

	virtual int mf_subscribe_all();  // 订阅所有渠道

	virtual int mf_add_subinfo(const char* stdinstid, int datatype);   // 添加待订阅的渠道

	virtual void open();    // 连接 

	virtual void mf_clear_depthfull();   // 清理 增量行情存储器

	std::string m_logfile;       // log 位置

	std::string m_resturl_base;  // http 请求baseurl 
	
	HttpClient* m_http_client;   // http client


};


#endif MYWEBSOCKETMARKET_H
