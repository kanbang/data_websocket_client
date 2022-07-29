#ifndef MYLOGGER_H
#define MYLOGGER_H


#include "MyStruct.h"
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <windows.h>

using namespace std;

class MyLogger
{

public:

	list<STR_LOG *> m_Log_List;
	CRITICAL_SECTION m_cs;
	int m_run;
	char m_logfile[1024];
	char m_dingfile[1024];

	MyLogger(const char * logfile = "");

	int mf_Add_Log(STR_LOG * loginfo);
	int mf_Add_Log(const char * logstr, int level = 0);
	int mf_Add_Log(const char * logstr, const char * filepath, int level = 0);
	STR_LOG * mf_Get_Log();


	int mf_Run_Write_Log();

	int mf_Write_Text(STR_LOG * loginfo);

	void mf_buildtime(STR_MYTIME *buildtime);
	
};

#endif