#include "MyWebSocketMarket.h"
#include "hv/WebSocketClient.h"


#pragma comment(lib,"hv")

MyWebSocketMarket::MyWebSocketMarket(std::string clientid)
{
	InitializeCriticalSection(&m_cs_subscribe);
	InitializeCriticalSection(&m_cs_update_depth);        // 深度更新锁

	m_wsid = clientid;

	m_wsurl = "";

	m_onmessage_count = 0;
	m_pingpong_ts = 0;
	m_onmessage_ts = 0;
	m_recvcount_depth = 0;
	m_recvcount_trade = 0;

	m_startping = 0;
	m_connect_status = 0;
	
	reconn_setting_init(&m_reconn);
	m_reconn.min_delay = 1000;
	m_reconn.max_delay = 10000;
	m_reconn.delay_policy = 2;
	m_ws.setReconnect(&m_reconn);
	
	m_http_client = new HttpClient();

	m_ws.onopen = [this]() {
		this->on_open();
		//printf("onopen1\n");
	};

	m_ws.onclose = [this]() {
		this->on_close();
		//printf("onclose\n");
	};

	m_ws.onmessage = [this](const std::string& msg) {
		this->on_message(msg);
		//printf("onmessage: %s end \n", msg.c_str());
	};

}


MyWebSocketMarket::MyWebSocketMarket()
{


}


MyWebSocketMarket::~MyWebSocketMarket()
{


}

int MyWebSocketMarket::m_thread_ping()
{

	return 0;
}


void MyWebSocketMarket::on_open()
{
	m_connect_status = 1;	
	printf("onopen1\n");
	//std::string mystr = "{ \"method\": \"SUBSCRIBE\",\"params\" :[\"ethusdt@aggTrade\",\"ethusdt@depth\"] ,\"id\" : 1}";
	//m_ws.send(mystr);
}


void MyWebSocketMarket::on_close()
{
	m_connect_status = 0;
	printf("onclose\n");
}


void MyWebSocketMarket::on_message(const std::string& msg)
{
	printf("onmessage: %s end \n", msg.c_str());
	Sleep(1000);
}


void MyWebSocketMarket::open()
{	
	m_ws.open(m_wsurl.c_str(), m_headers);
}


int MyWebSocketMarket::mf_subscribe_single(const char* stdinstid, int datatype)
{
	EnterCriticalSection(&m_cs_subscribe);




	LeaveCriticalSection(&m_cs_subscribe);

	return 0;
}



int MyWebSocketMarket::mf_subscribe_all()
{
	EnterCriticalSection(&m_cs_subscribe);



	LeaveCriticalSection(&m_cs_subscribe);

	m_connect_status = 2; // 完成所有订阅

	return 0;
}

int MyWebSocketMarket::mf_subscribe_depth_limit_all()
{
	EnterCriticalSection(&m_cs_subscribe);



	LeaveCriticalSection(&m_cs_subscribe);

	m_connect_status = 2; // 完成所有订阅

	return 0;
}

int MyWebSocketMarket::mf_subscribe_depth_full_all()
{
	EnterCriticalSection(&m_cs_subscribe);



	LeaveCriticalSection(&m_cs_subscribe);

	m_connect_status = 2; // 完成所有订阅

	return 0;
}


int MyWebSocketMarket::mf_unsubscribe_all_depth_limit()
{

	return 0;
}


int MyWebSocketMarket::mf_unsubscribe_all_depth_full()
{

	return 0;
}


int MyWebSocketMarket::mf_add_subinfo(const char* stdinstid, int datatype)
{
	EnterCriticalSection(&m_cs_subscribe);

	if (datatype == 1)
	{
		m_map_subinfo_depth[stdinstid] = 1;
	}
	else if (datatype == 2)
	{
		m_map_subinfo_trade[stdinstid] = 1;
	}

	LeaveCriticalSection(&m_cs_subscribe);

	return 0;
}


void MyWebSocketMarket::mf_clear_depthfull()
{
	EnterCriticalSection(&m_cs_update_depth);

	m_map_depthfull_bid.clear();
	m_map_depthfull_ask.clear();

	LeaveCriticalSection(&m_cs_update_depth);

}