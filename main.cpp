#include<stdio.h>
#include<stdlib.h>

#include<string>
#include<vector>
#include<list>
#include<map>
#include<thread>
#include <objbase.h>
#include <string.h>  
#include <iostream> 

#include "tools/MyZmqPublish.h"
#include "tools/MyLogger.h"

#include "clients/MyWebSocketMarket_Binance_f.h"
#include "clients/MyWebSocketMarket_Okx_f.h"

using namespace std;

using namespace rapidjson;
using namespace std;

Document* gcp_config = NULL;

MyLogger *gcp_mylogger = NULL;

std::map<std::string, MyZmqPublish*> gcp_map_zmqpublish;


thread * gcp_thread_zmqconsume[30];


std::vector<MyWebSocketMarket*> gcp_vec_mywebsocktmarket;


/////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////


int gf_split3(const std::string& str, std::vector<std::string>& ret_, std::string sep)
{
	ret_.clear();
	if (str.empty())
	{
		return 0;
	}

	int strcount = 0;
	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;

	while (pos_begin != std::string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != std::string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.push_back(tmp);
			strcount++;
			tmp.clear();
		}
	}
	return strcount;
}


long long gf_getlltime()
{
	static long long begints = 0;

	if (begints == 0)
	{
		SYSTEMTIME currentTime;
		GetLocalTime(&currentTime);
		tm temptm = { currentTime.wSecond,
		currentTime.wMinute,
		currentTime.wHour,
		currentTime.wDay,
		currentTime.wMonth - 1,
		currentTime.wYear - 1900
		};
		long long nTimer = mktime(&temptm) * 1000 + currentTime.wMilliseconds;

		begints = nTimer - GetTickCount64();
	}
	return GetTickCount64() + begints;
}



////////////////////////////////////////// 
////////////////////////////////////////// 
////////////////////////////////////////// 


int load_config(const char* jsonfile)
{
	std::ifstream in;
	std::string stringFromStream;
	in.open(jsonfile, std::ifstream::in);
	if (!in.is_open())
		return -1;
	std::string line;
	while (getline(in, line)) {
		stringFromStream.append(line + "\n");
	}
	in.close();

	Document * config = new Document();
	config->Parse(stringFromStream.c_str());

	if (gcp_config != NULL)
	{
		Document * temp = gcp_config;
		delete temp;	
	}
	
	gcp_config = config;

	return  0;
}




MyWebSocketMarket* create_websocket_market(const char* platformname, const char* wsid)
{
	MyWebSocketMarket* ws = NULL;

	if (strcmp(platformname, "binance-f") == 0)
	{
		ws = new MyWebSocketMarket_Binance_f(wsid);
	}
	else if (strcmp(platformname, "okx-f") == 0)
	{
		ws = new MyWebSocketMarket_Okx_f(wsid);
	}
	
	return ws;
}



int init_market()
{
	const Value& WSMarketConfig = (*gcp_config)["WSMarketConfig"];
	for (Value::ConstMemberIterator itr = WSMarketConfig.MemberBegin(); itr != WSMarketConfig.MemberEnd(); ++itr)
	{
		std::string wsid = itr->name.GetString();
		std::string wsclassname = itr->value["wsclass"].GetString();

		MyWebSocketMarket* newws = create_websocket_market(wsclassname.c_str(), wsid.c_str());

		gcp_vec_mywebsocktmarket.push_back(newws);

		std::string cnstr = itr->value["zmqstr"].GetString();
		gcp_map_zmqpublish[wsid] = new MyZmqPublish();
		gcp_map_zmqpublish[wsid]->mf_SetCon(cnstr);

		for (Value::ConstMemberIterator subitr = itr->value.MemberBegin(); subitr != itr->value.MemberEnd(); ++subitr)
		{
			std::string channelname = subitr->name.GetString();

			if (channelname == "depth")
			{
				const Value& symbolarray = subitr->value["symbol"].GetArray();
				size_t symbolsize = symbolarray.Size();
				sscanf(subitr->value["level"].GetString(), "%d", &newws->m_subinfo_depth_level);

				if (strcmp(subitr->value["type"].GetString(), "limit") == 0)
				{
					newws->m_subinfo_depth_type = 1;
				}
				else if (strcmp(subitr->value["type"].GetString(), "full") == 0)
				{
					newws->m_subinfo_depth_type = 2;
				}

				for (int i = 0; i < symbolsize; i++)
				{
					string fullsymbolname = wsclassname + "_" + symbolarray[i].GetString();
					newws->mf_add_subinfo(fullsymbolname.c_str(),1);
				}
			}
			else if (channelname == "trade")
			{
				const Value& symbolarray = subitr->value["symbol"].GetArray();
				size_t symbolsize = symbolarray.Size();
				for (int i = 0; i < symbolsize; i++)
				{
					string fullsymbolname = wsclassname + "_" + symbolarray[i].GetString();
					newws->mf_add_subinfo(fullsymbolname.c_str(), 2);
				}
			}
		}

	}

	return 0;
}


int start_websocket_market()
{
	for (int i = 0; i < gcp_vec_mywebsocktmarket.size(); i++)
	{
		gcp_vec_mywebsocktmarket[i]->open();
	}

	return gcp_vec_mywebsocktmarket.size();

}


int start_websocket_zmq()
{
	for (auto itor = gcp_map_zmqpublish.begin(); itor != gcp_map_zmqpublish.end(); itor++)
	{
		itor->second->mf_Connect();
	}
	
	return gcp_map_zmqpublish.size();
}




int main()
{
	
	long long nowts = gf_getlltime();

	// 创建日志对象
	gcp_mylogger = new MyLogger();//创建日志对象

	thread mylogger = thread(&MyLogger::mf_Run_Write_Log, gcp_mylogger);

	// 加载配置文件
	load_config("./config/myconfig.json");

	init_market();

	start_websocket_market();

	start_websocket_zmq();

	while (true)
	{
		Sleep(100000);
	}
}

