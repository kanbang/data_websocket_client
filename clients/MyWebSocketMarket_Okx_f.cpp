#include "MyWebSocketMarket_Okx_f.h"
#include "../tools/MyStruct.h"
#include "../tools/MyLogger.h"
#include "../tools/MyZmqPublish.h"


MyWebSocketMarket_Okx_f::MyWebSocketMarket_Okx_f(std::string clientid) :MyWebSocketMarket(clientid)
{
	m_subscribe_id = 0;
	m_wsurl = "wss://ws.okx.com:8443/ws/v5/public";
	m_logfile = "./LOG/marketlog_" + clientid + ".txt";

	c_needflash_depthfull = 0;
}



MyWebSocketMarket_Okx_f::MyWebSocketMarket_Okx_f()
{

}



MyWebSocketMarket_Okx_f::~MyWebSocketMarket_Okx_f()
{

}



void MyWebSocketMarket_Okx_f::on_open()
{
	gcp_mylogger->mf_Add_Log("okx_f on_open", m_logfile.c_str());
	m_connect_status = 1;
	mf_subscribe_all();

	if (m_startping == 0)
	{
		m_startping = 1;
		thread* newthread = new thread(&MyWebSocketMarket_Okx_f::m_thread_ping, this);
	}
}



int MyWebSocketMarket_Okx_f::m_thread_ping()
{
	while (m_startping)
	{
		long long nowts = gf_getlltime();
		if (c_needflash_depthfull == 1)
		{
			mf_unsubscribe_depth_all();
			Sleep(2000);
			mf_subscribe_depth_all();
			c_needflash_depthfull = 0;
		}
		
		if (nowts - m_pingpong_ts > 10000)
		{
			if (m_ws.isConnected())
			{
				m_ws.send("ping");
				m_pingpong_ts = nowts;
				//mf_restreq_myaccount();
			}
		}
		
		Sleep(2000);
	}

	return 0;
}



void MyWebSocketMarket_Okx_f::on_close()
{
	gcp_mylogger->mf_Add_Log("okx_f on_close", m_logfile.c_str());
	m_connect_status = -1;
}





int MyWebSocketMarket_Okx_f::mf_subscribe_all()
{
	mf_subscribe_depth_all();
	mf_subscribe_trade_all();

	m_connect_status = 2; // 完成所有订阅

	return 0;
}




int MyWebSocketMarket_Okx_f::mf_subscribe_depth_all()
{
	mf_clear_depthfull();
	
	if (m_subinfo_depth_type == 1)
	{
		if (m_subinfo_depth_level == 1)
		{
			cf_sub_unsub_all(1, 1, 1, 1);
		}
		else
		{
			cf_sub_unsub_all(1, 1, 1, 5);
		}
	}
	else if (m_subinfo_depth_type == 2)
	{
		if (m_subinfo_depth_level == 1)
		{
			cf_sub_unsub_all(1, 1, 2, 400);
		}
		else
		{
			cf_sub_unsub_all(1, 1, 2, 400);
		}
	}

	return 0;
}



int MyWebSocketMarket_Okx_f::mf_subscribe_trade_all()
{
	cf_sub_unsub_all(1, 2, 1, 1);

	return 0;
}




int MyWebSocketMarket_Okx_f::mf_unsubscribe_depth_all()
{
	
	if (m_subinfo_depth_type == 1)
	{
		if (m_subinfo_depth_level == 1)
		{
			cf_sub_unsub_all(2, 1, 1, 1);
		}
		else
		{
			cf_sub_unsub_all(2, 1, 1, 5);
		}
	}
	else if (m_subinfo_depth_type == 2)
	{
		if (m_subinfo_depth_level == 1)
		{
			cf_sub_unsub_all(2, 1, 2, 400);
		}
		else
		{
			cf_sub_unsub_all(2, 1, 2, 400);
		}
	}

	return 0;
}


int MyWebSocketMarket_Okx_f::mf_unsubscribe_trade_all()
{
	cf_sub_unsub_all(2, 2, 1, 1);

	return 0;
}



int MyWebSocketMarket_Okx_f::cf_sub_unsub_all(int suborunsub, int subchannel, int limitorfull, int level)
{
	EnterCriticalSection(&m_cs_subscribe);

	std::string myop = "";
	std::string mycnl = "";
	std::string mylimitfull = "";

	std::string opstr = "";

	std::map<std::string,int>* p_subinfo = NULL;

	if (suborunsub == 1)
	{
		myop = "subscribe";
	}
	else if (suborunsub == 2)
	{
		myop = "unsubscribe";
	}

	if (subchannel == 1)
	{
		p_subinfo = &m_map_subinfo_depth;
		if (limitorfull == 1)
		{
			if (level == 1)
			{
				mycnl = "bbo-tbt";
			}
			else if (level == 5)
			{
				mycnl = "books5";
			}
			else
			{
				mycnl = "books5";
			}		
		}
		else if (limitorfull == 2)
		{
			mycnl = "books";
		}	
	}
	else if (subchannel == 2)
	{
		p_subinfo = &m_map_subinfo_trade;
		mycnl = "trades";
	}
	
	int channelsum = 0;
	int index = 0;
	int channelcount = 0;
	Document doc[100];

	Value args(rapidjson::kArrayType);
	doc[index].SetObject();
	Document::AllocatorType& allocator = doc[index].GetAllocator();
	doc[index].AddMember("args", args, allocator);


	for (auto itr = p_subinfo->begin(); itr != p_subinfo->end(); itr++)
	{
		Document::AllocatorType& allocator = doc[index].GetAllocator();

		string cnlinstid = mf_stdinstid2cnlinstid(itr->first.c_str());
		if (cnlinstid != "")
		{
			channelcount++;
			channelsum++;

			Value element(rapidjson::kObjectType);
			Value channel(rapidjson::kStringType);
			Value instId(rapidjson::kStringType);

			channel.SetString(mycnl.c_str(), allocator);
			instId.SetString(cnlinstid.c_str(), allocator);

			element.AddMember("channel", channel, allocator);
			element.AddMember("instId", instId, allocator);

			doc[index]["args"].PushBack(element, allocator);
		}

		if (channelcount % 50 == 0 && channelcount > 0)
		{
			Value op(rapidjson::kStringType);

			op.SetString(myop.c_str(), allocator);
			doc[index].AddMember("op", op, allocator);

			StringBuffer buffer;
			Writer<StringBuffer> write(buffer);
			doc[index].Accept(write);
			std::string json = buffer.GetString();


			opstr = "okx_f mf_sub_unsub_all " + myop + " " + mycnl + " " + std::to_string(channelsum);
			gcp_mylogger->mf_Add_Log(opstr.c_str(), m_logfile.c_str());

			m_ws.send(json);

			Sleep(1000);

			index++;
			channelcount = 0;
			doc[index].SetObject();
			Value args(rapidjson::kArrayType);
			doc[index].AddMember("args", args, allocator);
		}
	}

	if (channelcount > 0)
	{
		Document::AllocatorType& allocator = doc[index].GetAllocator();

		Value op(rapidjson::kStringType);

		op.SetString(myop.c_str(), allocator);

		doc[index].AddMember("op", op, allocator);

		StringBuffer buffer;
		Writer<StringBuffer> write(buffer);
		doc[index].Accept(write);
		std::string json = buffer.GetString();

		opstr = "okx_f mf_sub_unsub_all " + myop + " " + mycnl + " " + std::to_string(channelsum);
		gcp_mylogger->mf_Add_Log(opstr.c_str(), m_logfile.c_str());

		m_ws.send(json);
	}

	LeaveCriticalSection(&m_cs_subscribe);

	return channelsum;
}




std::string MyWebSocketMarket_Okx_f::mf_stdinstid2cnlinstid(const char* stdinstid)
{
	string stdstr = stdinstid;
	size_t findpos = stdstr.find("okx-f_");
	if (findpos >= 0)
	{
		string cnlstr = stdstr.replace(findpos, 6, "");
		cnlstr += "-SWAP";
		return cnlstr;
	}

	return "";
}



std::string MyWebSocketMarket_Okx_f::mf_cnlinstid2stdinstid(const char* cnlinstid)
{
	auto itor = m_map_cnl2std.find(cnlinstid);
	if (itor == m_map_cnl2std.end())
	{
		string cnlstr = cnlinstid;
		size_t findpos = cnlstr.find("-SWAP");
		if (findpos >= 0)
		{
			string stdinstid = "okx-f_" + cnlstr.replace(findpos, 5, "");
			m_map_cnl2std[cnlinstid] = stdinstid;
			return stdinstid;
		}	
	}
	else
	{
		return itor->second;
	}

	return "";
}



void MyWebSocketMarket_Okx_f::on_message(const std::string& msg)
{
	//printf("%s\n", msg.c_str());
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

	if (doc.HasMember("arg"))
	{
		if (strstr(doc["arg"]["channel"].GetString(), "books") != NULL && doc.HasMember("data"))
		{
			m_recvcount_depth++;
			mf_parse_depth(doc, m_onmessage_ts);
		}else if (strcmp(doc["arg"]["channel"].GetString(), "trades") == 0 && doc.HasMember("data"))
		{
			m_recvcount_trade++;
			mf_parse_trade(doc, m_onmessage_ts);
		}
	}

	if (m_onmessage_count % 100000 == 0)
	{
		gcp_mylogger->mf_Add_Log(msg.c_str(), m_logfile.c_str());
	}

}
 


int MyWebSocketMarket_Okx_f::mf_parse_depth(Document& doc, long long receivets)
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



int MyWebSocketMarket_Okx_f::mf_parse_depth_limit(Document& doc, long long receivets)
{
	long long exchangets = 0;
	sscanf(doc["data"][0]["ts"].GetString(), "%lld", &exchangets);

	Value& askdata = doc["data"][0]["asks"];
	Value& biddata = doc["data"][0]["bids"];

	string symbol = doc["arg"]["instId"].GetString();
	string stdinstid = mf_cnlinstid2stdinstid(symbol.c_str());
	
	
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("instrument_id");
	writer.String(stdinstid.c_str());

	writer.Key("data_type");
	writer.String("depth");

	writer.Key("exchange");
	writer.String("OKX_FUTURES");

	writer.Key("symbol");
	writer.String(symbol.c_str());

	writer.Key("timestamp");
	writer.Int64(exchangets);

	writer.Key("receipt_timestamp");
	writer.Int64(receivets);

	writer.Key("book");
	writer.StartObject();

	writer.Key("bid");
	writer.StartObject();

	size_t bidsize = biddata.Size();
	for (int i = 0; i < bidsize; i++)
	{
		writer.Key(biddata[i][0].GetString());
		double price = 0;
		sscanf(biddata[i][1].GetString(), "%lf", &price);
		writer.Double(price);
	}

	writer.EndObject();

	writer.Key("ask");
	writer.StartObject();

	size_t asksize = askdata.Size();
	for (int i = 0; i < asksize; i++)
	{
		writer.Key(askdata[i][0].GetString());
		double price = 0;
		sscanf(askdata[i][1].GetString(), "%lf", &price);
		writer.Double(price);
	}

	writer.EndObject();

	writer.EndObject();

	writer.EndObject();

	std::string mymsg = s.GetString();

	gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());


	return 0;
}




int MyWebSocketMarket_Okx_f::mf_parse_depth_full(Document& doc, long long receivets)
{
	EnterCriticalSection(&m_cs_update_depth);
	
	long long exchangets = 0;
	sscanf(doc["data"][0]["ts"].GetString(), "%lld", &exchangets);

	Value& askdata = doc["data"][0]["asks"];
	Value& biddata = doc["data"][0]["bids"];

	string symbol = doc["arg"]["instId"].GetString();
	char stdinstid[1024];
	strcpy(stdinstid,mf_cnlinstid2stdinstid(symbol.c_str()).c_str());


	double price = 0;
	double volume = 0;

	size_t arraysize = askdata.Size();
	std::map<double, STR_MYDEPTHFULL, StrCompare_Asc>& mymap_ask = m_map_depthfull_ask[stdinstid];

	for (int i = 0; i < arraysize; i++)
	{
		sscanf(askdata[i][0].GetString(), "%lf", &price);
		sscanf(askdata[i][1].GetString(), "%lf", &volume);

		if (volume > H_MINVALUE)
		{
			if (mymap_ask[price].vol == 0) // 本档第一次赋值
			{
				strcpy(mymap_ask[price].pricestr, askdata[i][0].GetString());
			}
			mymap_ask[price].vol = volume;
			mymap_ask[price].updatets = exchangets;
			strcpy(mymap_ask[price].volstr, askdata[i][1].GetString());
		}
		else
		{
			mymap_ask.erase(price);
		}	
	}

	arraysize = biddata.Size();
	std::map<double, STR_MYDEPTHFULL, StrCompare_Des>& mymap_bid = m_map_depthfull_bid[stdinstid];

	for (int i = 0; i < arraysize; i++)
	{
		sscanf(biddata[i][0].GetString(), "%lf", &price);
		sscanf(biddata[i][1].GetString(), "%lf", &volume);

		if (volume > H_MINVALUE)
		{
			if (mymap_bid[price].vol == 0) // 本档第一次赋值
			{
				strcpy(mymap_bid[price].pricestr, biddata[i][0].GetString());
			}
			mymap_bid[price].vol = volume;
			mymap_bid[price].updatets = exchangets;		
			strcpy(mymap_bid[price].volstr, biddata[i][1].GetString());
		}
		else
		{
			mymap_bid.erase(price);
		}
	}


	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("instrument_id");
	writer.String(stdinstid);

	writer.Key("data_type");
	writer.String("depth");

	writer.Key("exchange");
	writer.String("OKX_FUTURES");

	writer.Key("symbol");
	writer.String(symbol.c_str());

	writer.Key("timestamp");
	writer.Int64(exchangets);

	writer.Key("receipt_timestamp");
	writer.Int64(receivets);

	writer.Key("book");
	writer.StartObject();

	writer.Key("bid");
	writer.StartObject();


	char pricestr[1024];
	char logstr[1024];

	double bid0 = 0;
	int count = 0;
	size_t bidsize = min(mymap_bid.size(),m_subinfo_depth_level);
	//size_t bidsize = min(mymap_bid.size(), 1);
	for (auto itor = mymap_bid.begin(); itor != mymap_bid.end(); itor++)
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

	LeaveCriticalSection(&m_cs_update_depth);

	if (bidsize > 0 && asksize > 0)
	{
		if (ask0 > bid0)
		{
			gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());

			/*
			if (strcmp(stdinstid, "okx-f_UNI-USDT") == 0)
			{
				printf("%s\n >>>%lld", mymsg.c_str(), m_recvcount_depth);
			}*/
		}
		else
		{
			sprintf(logstr, "okx_f mf_parse_depth_full error %s", stdinstid);
			gcp_mylogger->mf_Add_Log(logstr, m_logfile.c_str());
			c_needflash_depthfull = 1;
		}
	}

	
	if (m_recvcount_depth % 10 == 0)
	{
		bool checkret = cf_check_depthfull(stdinstid, doc["data"][0]["checksum"].GetInt());
		if (checkret == false)
		{
			sprintf(logstr, "okx_f cf_check_depthfull error %s", stdinstid);
			gcp_mylogger->mf_Add_Log(logstr, m_logfile.c_str());
			c_needflash_depthfull = 1;
		}
	}
	

	/*
	for (int i = 0; i < 1000000; i++)
	{
		cf_check_depthfull(stdinstid, doc["data"][0]["checksum"].GetInt());
	}*/

	//gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());

	return 0;
}



int MyWebSocketMarket_Okx_f::mf_parse_trade(Document& doc, long long receivets)
{
	Value& mydata = doc["data"][0];
	
	long long exchangets = 0;
	sscanf(mydata["ts"].GetString(), "%lld", &exchangets);

	string symbol = doc["arg"]["instId"].GetString();
	string stdinstid = mf_cnlinstid2stdinstid(symbol.c_str());


	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key("instrument_id");
	writer.String(stdinstid.c_str());

	writer.Key("data_type");
	writer.String("trade");

	writer.Key("exchange");
	writer.String("OKX_FUTURES");

	writer.Key("symbol");
	writer.String(symbol.c_str());

	writer.Key("timestamp");
	writer.Int64(exchangets);

	writer.Key("receipt_timestamp");
	writer.Int64(receivets);

	writer.Key("id");
	writer.String(mydata["tradeId"].GetString());

	writer.Key("side");
	writer.String(mydata["side"].GetString());


	writer.Key("price");
	double price = 0;
	sscanf(mydata["px"].GetString(), "%lf", &price);
	writer.Double(price);


	writer.Key("amount");
	double amount = 0;
	sscanf(mydata["sz"].GetString(), "%lf", &amount);
	writer.Double(amount);

	writer.EndObject();

	std::string mymsg = s.GetString();

	gcp_map_zmqpublish[m_wsid]->mf_send(mymsg.c_str());

	/*
	if (strcmp(stdinstid.c_str(), "okx-f_BTC-USDT") == 0)
	{
		printf("%s\n >>>%lld", mymsg.c_str(), m_recvcount_trade);
	}*/

	return 0;
}



const UINT32 table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


DWORD MyWebSocketMarket_Okx_f::cf_getCRC(BYTE* ptr, DWORD Size)
{
	// 计算CRC32值
	DWORD crcTmp2 = 0xFFFFFFFF;
	while (Size--)
	{
		crcTmp2 = ((crcTmp2 >> 8) & 0x00FFFFFF) ^ table[(crcTmp2 ^ (*ptr)) & 0xFF];
		ptr++;
	}
	return (crcTmp2 ^ 0xFFFFFFFF);
}


bool MyWebSocketMarket_Okx_f::cf_check_depthfull(const char* stdinstid, UINT32 excrc)
{
	std::map<double, STR_MYDEPTHFULL, StrCompare_Asc>& mymap_ask = m_map_depthfull_ask[stdinstid];
	std::map<double, STR_MYDEPTHFULL, StrCompare_Des>& mymap_bid = m_map_depthfull_bid[stdinstid];

	char checkstr[10240];

	char * ap[30];
	char * av[30];
	char * bp[30];
	char * bv[30];


	if (mymap_ask.size() > 25 && mymap_bid.size() > 25)
	{
		int count = 0;
		for (auto itr = mymap_ask.begin(); itr != mymap_ask.end(); itr++)
		{
			ap[count] = itr->second.pricestr;
			av[count] = itr->second.volstr;
			count++;
			if (count == 25) break;
		}

		count = 0;
		for (auto itr = mymap_bid.begin(); itr != mymap_bid.end(); itr++)
		{
			bp[count] = itr->second.pricestr;
			bv[count] = itr->second.volstr;
			count++;
			if (count == 25) break;
		}

		sprintf(checkstr,
			"%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s",
			bp[0], bv[0], ap[0], av[0],
			bp[1], bv[1], ap[1], av[1],
			bp[2], bv[2], ap[2], av[2],
			bp[3], bv[3], ap[3], av[3],
			bp[4], bv[4], ap[4], av[4],
			bp[5], bv[5], ap[5], av[5],
			bp[6], bv[6], ap[6], av[6],
			bp[7], bv[7], ap[7], av[7],
			bp[8], bv[8], ap[8], av[8],
			bp[9], bv[9], ap[9], av[9],
			bp[10], bv[10], ap[10], av[10],
			bp[11], bv[11], ap[11], av[11],
			bp[12], bv[12], ap[12], av[12],
			bp[13], bv[13], ap[13], av[13],
			bp[14], bv[14], ap[14], av[14],
			bp[15], bv[15], ap[15], av[15],
			bp[16], bv[16], ap[16], av[16],
			bp[17], bv[17], ap[17], av[17],
			bp[18], bv[18], ap[18], av[18],
			bp[19], bv[19], ap[19], av[19],
			bp[20], bv[20], ap[20], av[20],
			bp[21], bv[21], ap[21], av[21],
			bp[22], bv[22], ap[22], av[22],
			bp[23], bv[23], ap[23], av[23],
			bp[24], bv[24], ap[24], av[24]);
	}
	else
	{
		return true;
	}

	/*
	for (int i = 0; i < 1000000; i++)
	{
		UINT32 ss = cf_getCRC((unsigned char*)checkstr, strlen(checkstr));
	}*/
	

	if (cf_getCRC((unsigned char*)checkstr, strlen(checkstr)) == excrc)
	{
		return true;
	}

	return false;
}




