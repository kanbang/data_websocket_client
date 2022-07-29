#include "MyWebSocketMarket_Binance_f.h"
#include "MyStruct.h"
#include "MyMarket.h"
#include "MyLogger.h"
#include "MyZmqPublish.h"

MyWebSocketMarket_Binance_f::MyWebSocketMarket_Binance_f(std::string clientid):MyWebSocketMarket(clientid)
{
	m_subscribe_id = 0;
	m_wsurl = "wss://fstream.binance.com/ws/";
	m_logfile = "./LOG/marketlog_"+ clientid + ".txt";
	m_resturl_base = "https://fapi.binance.com/";

	c_needflash_depthfull = 1;
}



MyWebSocketMarket_Binance_f::MyWebSocketMarket_Binance_f()
{

}



MyWebSocketMarket_Binance_f::~MyWebSocketMarket_Binance_f()
{

}



void MyWebSocketMarket_Binance_f::on_open()
{
	gcp_mylogger->mf_Add_Log("binance_f market on_open", m_logfile.c_str());
	m_connect_status = 1;	
	mf_subscribe_all();

	if (m_startping == 0)
	{
		m_startping = 1;
		thread* newthread = new thread(&MyWebSocketMarket_Binance_f::m_thread_ping, this);
	}
}



int MyWebSocketMarket_Binance_f::m_thread_ping()
{
	while (m_startping)
	{
		long long nowts = gf_getlltime();
		if (c_needflash_depthfull == 1)
		{
			mf_clear_depthfull();
			cf_restreq_all_depthimage(m_subinfo_depth_level);
			Sleep(2000);
			c_needflash_depthfull = 0;
		}
		
		if (nowts - m_pingpong_ts > 10000)
		{
			if (m_ws.isConnected())
			{
				m_ws.send("pong");
				gcp_mylogger->mf_Add_Log("pong", m_logfile.c_str());
				m_pingpong_ts = nowts;
				//mf_restreq_myaccount();
			}
		}
		Sleep(2000);
	}

	return 0;
}


void MyWebSocketMarket_Binance_f::on_close()
{
	gcp_mylogger->mf_Add_Log("binance_f on_close", m_logfile.c_str());
	m_connect_status = -1;
}




int MyWebSocketMarket_Binance_f::mf_subscribe_all()
{
	mf_subscribe_depth_all();
	mf_subscribe_trade_all();
	m_connect_status = 2; // 完成所有订阅

	return 0;
}


int MyWebSocketMarket_Binance_f::mf_subscribe_depth_all()
{
	if (m_subinfo_depth_type == 1)
	{
		cf_sub_unsub_all(1, 1, 1, m_subinfo_depth_level);
	}
	else if (m_subinfo_depth_type == 2)
	{
		cf_sub_unsub_all(1, 1, 2, 400);
	}

	return 0;
}


int MyWebSocketMarket_Binance_f::mf_subscribe_trade_all()
{
	cf_sub_unsub_all(1, 2, 1, 1);

	return 0;
}


int MyWebSocketMarket_Binance_f::cf_restreq_all_depthimage(int level)
{

	for (auto itr = m_map_subinfo_depth.begin(); itr != m_map_subinfo_depth.end(); itr++)
	{
		HttpRequest req;

		std::string myts = std::to_string(gf_getlltime());

		std::string cnlinstid = mf_stdinstid2cnlinstid(itr->first.c_str());
		std::string requestPath = "fapi/v1/depth";
		std::string queryString = "?symbol="+ cnlinstid+"&limit="+std::to_string(m_subinfo_depth_level);

		req.method = HTTP_GET;
		req.url = (m_resturl_base + requestPath + queryString).c_str();

		req.headers["ACCESS-TIMESTAMP"] = myts.c_str();
		req.headers["Connection"] = "Keep-Alive";
		//req->headers["Content-Type"] = "application/json";  //GET 不能有此项目
		req.headers["locale"] = "en-US";

		//req->body = "";

		req.timeout = 10;

		HttpResponse resp;
		int ret = m_http_client->send(&req, &resp);

		if (resp.status_code == HTTP_STATUS_OK)
		{
			Document doc;
			doc.Parse(resp.body.c_str());
			if (doc.HasParseError() == false)
			{
				cf_restparse_all_depthimage(itr->first.c_str(),doc);
			}

			/*
			if (ret != 0) {
				printf("request failed!\n");
			}
			else {
				printf("%d %s\r\n", resp.status_code, resp.status_message());
				printf("%s\n", resp.body.c_str());
				Document doc;
				doc.Parse(resp.body.c_str());

				if (doc.HasParseError() == false)
				{
					mf_restparse_myassets(doc);
				}
			}*/
		}

		Sleep(50);
	}
	
	c_needflash_depthfull = 0;

	return 0;
}


int MyWebSocketMarket_Binance_f::cf_restparse_all_depthimage(const char* stdsymbol,Document& doc)
{
	
	EnterCriticalSection(&m_cs_update_depth);

	long long updatets = doc["T"].GetInt64();
	long long updateid = doc["lastUpdateId"].GetInt64();

	Value& myasks = doc["asks"];
	size_t mysize = myasks.Size();

	for (int i = 0; i < mysize; i++)
	{
		double price = atof(myasks[i][0].GetString());
		double vol = atof(myasks[i][1].GetString());

		std::map<double, STR_MYDEPTHFULL, StrCompare_Asc >& mymap = m_map_depthfull_ask[stdsymbol];

		if (mymap.count(price) == 0)
		{
			mymap[price].updateid = updateid;
			mymap[price].updatets = updatets;
			mymap[price].vol = vol;
		}
		else if(mymap[price].updateid< updateid)
		{
			mymap[price].updateid = updateid;
			mymap[price].updatets = updatets;
			mymap[price].vol = vol;
		}
	}

	Value& mybids = doc["bids"];
	mysize = mybids.Size();

	for (int i = 0; i < mysize; i++)
	{
		double price = atof(mybids[i][0].GetString());
		double vol = atof(mybids[i][1].GetString());

		std::map<double, STR_MYDEPTHFULL, StrCompare_Des >& mymap = m_map_depthfull_bid[stdsymbol];

		if (mymap.count(price) == 0)
		{
			mymap[price].updateid = updateid;
			mymap[price].updatets = updatets;
			mymap[price].vol = vol;
		}
		else if (mymap[price].updateid < updateid)
		{
			mymap[price].updateid = updateid;
			mymap[price].updatets = updatets;
			mymap[price].vol = vol;
		}
	}

	LeaveCriticalSection(&m_cs_update_depth);
	
	return 0;
}





int MyWebSocketMarket_Binance_f::cf_sub_unsub_all(int suborunsub, int subchannel, int limitorfull, int level)
{
	EnterCriticalSection(&m_cs_subscribe);


	std::string myop = "";
	std::string mycnl = "";
	std::string mylimitfull = "";

	std::string opstr = "";

	std::map<std::string,int>* p_subinfo = NULL;

	if (suborunsub == 1)
	{
		myop = "SUBSCRIBE";
	}
	else if (suborunsub == 2)
	{
		myop = "UNSUBSCRIBE";
	}

	if (subchannel == 1)
	{
		p_subinfo = &m_map_subinfo_depth;	
		if (limitorfull == 1)
		{
			if (level == 5 || level == 10 || level == 20)
			{
				mycnl = "@depth";
				mycnl += std::to_string(level);
			}
			else
			{
				mycnl = "@depth20";
			}		
		}
		else if (limitorfull == 2)
		{
			mycnl = "@depth";
		}
	}
	else if (subchannel == 2)
	{
		p_subinfo = &m_map_subinfo_trade;
		mycnl = "@aggTrade";
	}

	opstr = opstr + myop + mycnl + " ";

	int channelsum = 0;
	int index = 0;
	int channelcount = 0;
	Document doc[100];

	Value params(rapidjson::kArrayType);
	doc[index].SetObject();
	Document::AllocatorType& allocator = doc[index].GetAllocator();
	doc[index].AddMember("params", params, allocator);


	for (auto itr = p_subinfo->begin(); itr != p_subinfo->end(); itr++)
	{
		Document::AllocatorType& allocator = doc[index].GetAllocator();
		std::string channelwei = mycnl;

		string cnlinstid = mf_stdinstid2cnlinstid(itr->first.c_str());
		if (cnlinstid != "")
		{
			channelcount++;
			channelsum++;

			transform(cnlinstid.begin(), cnlinstid.end(), cnlinstid.begin(), ::tolower);
			cnlinstid += channelwei;
			Value element(rapidjson::kStringType);
			element.SetString(cnlinstid.c_str(), allocator);
			doc[index]["params"].PushBack(element, allocator);
		}


		if (channelcount % 30 == 0 && channelcount > 0)
		{
			Value method(rapidjson::kStringType);
			Value id(rapidjson::kNumberType);

			method.SetString(myop.c_str(), allocator);
			id.SetInt(m_subscribe_id++);

			doc[index].AddMember("method", method, allocator);
			doc[index].AddMember("id", id, allocator);

			StringBuffer buffer;
			Writer<StringBuffer> write(buffer);
			doc[index].Accept(write);
			std::string json = buffer.GetString();

			opstr = "binance_f mf_sub_unsub_all " + myop + " " + mycnl + " " + std::to_string(channelsum);
			gcp_mylogger->mf_Add_Log(opstr.c_str(), m_logfile.c_str());

			m_ws.send(json);

			Sleep(1000);

			index++;
			channelcount = 0;
			doc[index].SetObject();
			Value params(rapidjson::kArrayType);
			doc[index].AddMember("params", params, allocator);

		}
	}

	if (channelcount > 0)
	{
		Document::AllocatorType& allocator = doc[index].GetAllocator();

		Value method(rapidjson::kStringType);
		Value id(rapidjson::kNumberType);

		method.SetString(myop.c_str(), allocator);
		id.SetInt(m_subscribe_id++);

		doc[index].AddMember("method", method, allocator);
		doc[index].AddMember("id", id, allocator);

		StringBuffer buffer;
		Writer<StringBuffer> write(buffer);
		doc[index].Accept(write);
		std::string json = buffer.GetString();

		opstr = "binance_f mf_sub_unsub_all " + myop + " " + mycnl + " " + std::to_string(channelsum);
		gcp_mylogger->mf_Add_Log(opstr.c_str(), m_logfile.c_str());

		m_ws.send(json);
	}

	LeaveCriticalSection(&m_cs_subscribe);

	
	return channelsum;
}



std::string MyWebSocketMarket_Binance_f::mf_stdinstid2cnlinstid(const char* stdinstid)
{
	string stdstr = stdinstid;
	size_t findpos = stdstr.find("-USDT");
	if (findpos >= 0)
	{
		string cnlstr = stdstr.replace(findpos, 5, "USDT");
		findpos = cnlstr.find("binance-f_");
		if (findpos >= 0)
		{
			cnlstr = cnlstr.replace(findpos, 10, "");

		}

		return cnlstr;
	}

	return "";
}


void MyWebSocketMarket_Binance_f::on_message(const std::string& msg)
{
	m_onmessage_count++;

	if (m_connect_status == 2)
	{
		m_connect_status = 3;
	}
	
	if (m_connect_status != 3) return;


	Document doc;
	doc.Parse(msg.c_str());
	if (doc.HasParseError() == true)
	{
		return;
	}

	m_onmessage_ts = gf_getlltime();

	if (doc.HasMember("e"))
	{
		if (strcmp(doc["e"].GetString(), "depthUpdate") == 0)
		{
			m_recvcount_depth++;
			mf_parse_depth(doc, m_onmessage_ts);
				
		}else if(strcmp(doc["e"].GetString(), "aggTrade") == 0)
		{
			m_recvcount_trade++;
			mf_parse_trade(doc, m_onmessage_ts);
		}
	}
	

	if (m_onmessage_ts - m_pingpong_ts > 120000)
	{
		m_ws.send("pong");
		m_pingpong_ts = m_onmessage_ts;
	}

	if (m_onmessage_count % 100000 == 0)
	{
		gcp_mylogger->mf_Add_Log(msg.c_str(), m_logfile.c_str());
	}
}



int MyWebSocketMarket_Binance_f::mf_parse_depth(Document& doc, long long receivets)
{
	if (m_subinfo_depth_type == 1)
	{
		mf_parse_depth_limit(doc, receivets);
	}
	else if (m_subinfo_depth_type == 2)
	{
		mf_parse_depth_full(doc, receivets);
	}

	return 0;
}



int MyWebSocketMarket_Binance_f::mf_parse_depth_limit(Document& doc, long long receivets)
{
	char instid[1024];

	sprintf(instid, "binance-f_%s", doc["s"].GetString());
	size_t replacebigen = strlen(instid) - 4;
	instid[replacebigen] = '-';
	instid[replacebigen + 1] = 'U';
	instid[replacebigen + 2] = 'S';
	instid[replacebigen + 3] = 'D';
	instid[replacebigen + 4] = 'T';
	instid[replacebigen + 5] = '\0';
	
	
	
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();
	
		writer.Key("instrument_id");
		writer.String(instid);

		writer.Key("data_type");
		writer.String("depth");

		writer.Key("exchange");
		writer.String("BINANCE_FUTURES");

		writer.Key("symbol");
		writer.String(doc["s"].GetString());

		writer.Key("timestamp");
		writer.Int64(doc["T"].GetInt64());

		writer.Key("receipt_timestamp");
		writer.Int64(receivets);

		writer.Key("book");
		writer.StartObject();
			
			writer.Key("bid");
			writer.StartObject();

				size_t bidsize = doc["b"].Size();
				for (int i = 0; i < bidsize; i++)
				{
					writer.Key(doc["b"][i][0].GetString());
					double price = 0;
					sscanf(doc["b"][i][1].GetString(), "%lf", &price);
					writer.Double(price);
				}

			writer.EndObject();

			writer.Key("ask");
			writer.StartObject();

				size_t asksize = doc["a"].Size();
				for (int i = 0; i < asksize; i++)
				{
					writer.Key(doc["a"][i][0].GetString());
					double price = 0;
					sscanf(doc["a"][i][1].GetString(), "%lf", &price);
					writer.Double(price);
				}

			writer.EndObject();

		writer.EndObject();

	writer.EndObject();

	std::string mymsg = s.GetString();
	
	gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());
	

	return 0;
}




int MyWebSocketMarket_Binance_f::mf_parse_depth_full(Document& doc, long long receivets)
{
	
	EnterCriticalSection(&m_cs_update_depth);

	char logstr[1024];

	long long updatets = doc["T"].GetInt64();
	long long updateid = doc["u"].GetInt64();
	long long updateidpu = doc["pu"].GetInt64();


	char instid[1024];

	sprintf(instid, "binance-f_%s", doc["s"].GetString());
	size_t replacebigen = strlen(instid) - 4;
	instid[replacebigen] = '-';
	instid[replacebigen + 1] = 'U';
	instid[replacebigen + 2] = 'S';
	instid[replacebigen + 3] = 'D';
	instid[replacebigen + 4] = 'T';
	instid[replacebigen + 5] = '\0';

	if (c_depthfull_lastu.count(instid) == 0)
	{
		c_depthfull_lastu[instid] = updateid;
	}
	else
	{
		if (c_depthfull_lastu[instid] != updateidpu)
		{
			sprintf(logstr, "binance_f mf_parse_depth_full lost data %s", instid);
			gcp_mylogger->mf_Add_Log(logstr, m_logfile.c_str());
			c_needflash_depthfull = 1;
		}
	}

	c_depthfull_lastu[instid] = updateid;



	Value& myasks = doc["a"];
	size_t mysize = myasks.Size();

	std::map<double, STR_MYDEPTHFULL, StrCompare_Asc >& mymap_ask = m_map_depthfull_ask[instid];

	for (int i = 0; i < mysize; i++)
	{
		double price = atof(myasks[i][0].GetString());
		double vol = atof(myasks[i][1].GetString());

		if (mymap_ask.count(price) == 0)
		{
			if (vol > H_MINVALUE)
			{
				mymap_ask[price].updateid = updateid;
				mymap_ask[price].updatets = updatets;
				mymap_ask[price].vol = vol;
			}
		}
		else if (mymap_ask[price].updateid < updateid)
		{
			if (vol > H_MINVALUE)
			{
				mymap_ask[price].updateid = updateid;
				mymap_ask[price].updatets = updatets;
				mymap_ask[price].vol = vol;
			}
			else
			{
				mymap_ask.erase(price);
			}	
		}
	}

	Value& mybids = doc["b"];
	mysize = mybids.Size();

	std::map<double, STR_MYDEPTHFULL, StrCompare_Des >& mymap_bid = m_map_depthfull_bid[instid];

	for (int i = 0; i < mysize; i++)
	{
		double price = atof(mybids[i][0].GetString());
		double vol = atof(mybids[i][1].GetString());		

		if (mymap_bid.count(price) == 0)
		{
			if (vol > H_MINVALUE)
			{
				mymap_bid[price].updateid = updateid;
				mymap_bid[price].updatets = updatets;
				mymap_bid[price].vol = vol;
			}		
		}
		else if (mymap_bid[price].updateid < updateid)
		{
			if (vol > H_MINVALUE)
			{
				mymap_bid[price].updateid = updateid;
				mymap_bid[price].updatets = updatets;
				mymap_bid[price].vol = vol;
			}
			else
			{
				mymap_bid.erase(price);
			}
		}
	}


	LeaveCriticalSection(&m_cs_update_depth);

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("instrument_id");
	writer.String(instid);

	writer.Key("data_type");
	writer.String("depth");

	writer.Key("exchange");
	writer.String("BINANCE_FUTURES");

	writer.Key("symbol");
	writer.String(doc["s"].GetString());

	writer.Key("timestamp");
	writer.Int64(updatets);

	writer.Key("receipt_timestamp");
	writer.Int64(receivets);

	writer.Key("book");
	writer.StartObject();

	writer.Key("bid");
	writer.StartObject();


	char pricestr[1024];


	double bid0 = 0;
	int count = 0;
	size_t bidsize = min(mymap_bid.size(),m_subinfo_depth_level);
	//size_t bidsize = min(mymap_bid.size(), 1);
	for (auto itor = mymap_bid.begin();itor!= mymap_bid.end();itor++)
	{
		sprintf(pricestr, "%lf", itor->first);
		writer.Key(pricestr);
		writer.Double(itor->second.vol);
		
		if (count == 0) bid0 = itor->first;

		count++;
		if (count == bidsize) break;
	}

	writer.EndObject();

	writer.Key("ask");
	writer.StartObject();

	double ask0 = 0;
	count = 0;
	size_t asksize = min(mymap_ask.size(), m_subinfo_depth_level);
	//size_t asksize = min(mymap_ask.size(), 1);
	for (auto itor = mymap_ask.begin(); itor != mymap_ask.end(); itor++)
	{
		sprintf(pricestr, "%lf", itor->first);
		writer.Key(pricestr);
		writer.Double(itor->second.vol);

		if (count == 0) ask0 = itor->first;

		count++;
		if (count == asksize) break;
	}

	writer.EndObject();

	writer.EndObject();

	writer.EndObject();

	std::string mymsg = s.GetString();


	if (bidsize > 0 && asksize > 0 && c_needflash_depthfull == 0)
	{
		if (ask0 > bid0)
		{
			gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());
			
			/*
			if (strcmp(instid, "binance-f_BTC-USDT") == 0)
			{
				printf("%s\n >>>%lld", mymsg.c_str(), m_recvcount_depth);
			}*/	
		}
		else
		{
			sprintf(logstr, "binance_f mf_parse_depth_full error %s", instid);
			gcp_mylogger->mf_Add_Log(logstr, m_logfile.c_str());
			c_needflash_depthfull = 1;
		}	
	}

	return 0;
}



int MyWebSocketMarket_Binance_f::mf_parse_trade(Document& doc, long long receivets)
{
	char instid[1024];

	sprintf(instid, "binance-f_%s", doc["s"].GetString());
	size_t replacebigen = strlen(instid) - 4;
	instid[replacebigen] = '-';
	instid[replacebigen + 1] = 'U';
	instid[replacebigen + 2] = 'S';
	instid[replacebigen + 3] = 'D';
	instid[replacebigen + 4] = 'T';
	instid[replacebigen + 5] = '\0';



	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("instrument_id");
	writer.String(instid);

	writer.Key("data_type");
	writer.String("trade");

	writer.Key("exchange");
	writer.String("BINANCE_FUTURES");

	writer.Key("symbol");
	writer.String(doc["s"].GetString());

	writer.Key("timestamp");
	writer.Int64(doc["T"].GetInt64());

	writer.Key("receipt_timestamp");
	writer.Int64(receivets);

	writer.Key("id");
	char id[1024];
	sprintf(id, "%lld", doc["a"].GetInt64());
	writer.String(id);

	writer.Key("side");
	if (doc["m"].GetBool() == true)
	{
		writer.String("buy");
	}
	else
	{
		writer.String("sell");
	}


	writer.Key("price");
	double price = 0;
	sscanf(doc["p"].GetString(), "%lf", &price);
	writer.Double(price);


	writer.Key("amount");
	double amount = 0;
	sscanf(doc["q"].GetString(), "%lf", &amount);
	writer.Double(amount);

	writer.EndObject();

	std::string mymsg = s.GetString();

	gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());


	return 0;
}






