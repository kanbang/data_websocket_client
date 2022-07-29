#ifndef STR_MYSTRUCT_H_
#define STR_MYSTRUCT_H_

#include <string.h>
#include <time.h>
#include <windows.h>
#include <map>
#include <list>
#include <string>
#include <thread>
#include <vector>
#include <minmax.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

//#include <minmax.h>

#define H_MAXSTRING_LEN 1024
#define H_MINVALUE 0.00000001
#define H_MAXVALUE 1000000000


#pragma warning(disable:4996)



class MyLogger;
class MyMySql;
class MyZmqPublish;
class MyMarket;


extern MyLogger *gcp_mylogger;
extern MyMarket * gcp_mymarket;

extern std::map<std::string, MyZmqPublish*> gcp_map_zmqpublish;


int gf_split3(const std::string& str, std::vector<std::string>& ret_, std::string sep);
int gf_uuid(char * buffer);
long long gf_getlltime();

///////////////////////////////////自定义时间格式
///////////////////////////////



// 日志结构
struct STR_LOG
{
	int level;
	char filepath[1024];
	char msg[5000];

	STR_LOG()
	{
		strcpy(filepath, "");
		strcpy(msg, "");
		level = 0;
	}
};



#endif