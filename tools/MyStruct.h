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
#define H_PATH_MYLOG  ".//LOG//MyLog.txt"

#pragma warning(disable:4996)



class MyLogger;
class MyMySql;
class MyZmqPublish;

extern MyLogger *gcp_mylogger;

extern std::map<std::string, MyZmqPublish*> gcp_map_zmqpublish;


int gf_split3(const std::string& str, std::vector<std::string>& ret_, std::string sep);
int gf_uuid(char * buffer);
long long gf_getlltime();

//
struct STR_MYTIME
{
	char mytime_str[2046];           //
	int mytime_int_sec;              //
	int mytime_int_mil;              //

	long long mytime_ll;            //
	SYSTEMTIME mytime_st;           //
	FILETIME mytime_ft_real;        //
	FILETIME mytime_ft_temp;        //
	ULARGE_INTEGER mytime_ui;       //

	STR_MYTIME()
	{
		mytime_int_sec = 0;
		mytime_int_mil = 0;
		mytime_ll = 0;
		strcpy(mytime_str, "");
	}
};


//
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