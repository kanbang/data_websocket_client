# include"MyLogger.h"



MyLogger::MyLogger(const char * logfile)
{
	InitializeCriticalSection(&m_cs);

	m_run = 0;

	if (strcmp(logfile,"")==0)
	{
		strcpy(m_logfile, H_PATH_MYLOG);
	}
	else
	{
		strcpy(m_logfile, logfile);
	}
};

int MyLogger::mf_Add_Log(STR_LOG * loginfo)
{
	EnterCriticalSection(&m_cs);

		m_Log_List.push_back(loginfo);

	LeaveCriticalSection(&m_cs);

	return 0;
}


int MyLogger::mf_Add_Log(const char * logstr, int level)
{
	printf("Log >> %s\n", logstr);
	STR_LOG * newlog = new STR_LOG();
	newlog->level = level;
	strcpy(newlog->msg, logstr);
	mf_Add_Log(newlog);

	return 0;
}



int MyLogger::mf_Add_Log(const char * logstr, const char * filepath, int level)
{
	printf("Log >> %s\n", logstr);
	STR_LOG * newlog = new STR_LOG();
	newlog->level = level;
	strcpy(newlog->msg, logstr);
	strcpy(newlog->filepath, filepath);
	mf_Add_Log(newlog);

	return 0;
}



STR_LOG * MyLogger::mf_Get_Log()
{
	STR_LOG * newlog = NULL;
	EnterCriticalSection(&m_cs);

		if (m_Log_List.empty() == false)
		{
			newlog = m_Log_List.front();
			m_Log_List.pop_front();
		}

	LeaveCriticalSection(&m_cs);

	return newlog;
}

int MyLogger::mf_Run_Write_Log()
{
	m_run = 1;

	while (m_run)
	{
		STR_LOG * newlog = mf_Get_Log();

		while (newlog != NULL)
		{
			mf_Write_Text(newlog);
			delete newlog;
			newlog = mf_Get_Log();
		}

		Sleep(200);
	}
	return 0;
}



int MyLogger::mf_Write_Text(STR_LOG * loginfo)
{

	STR_MYTIME mytime;

	mf_buildtime(&mytime);
	std::string logstr;
	logstr = mytime.mytime_str;
	logstr += ":";
	logstr += loginfo->msg;
	//printf("Log >> %s\n", logstr.c_str());
	if (strcmp(loginfo->filepath, "") == 0)
	{
		std::ofstream fout_data(m_logfile, std::ios::app);		
		fout_data << logstr << std::endl;
		fout_data.close();
	}
	else
	{
		std::ofstream fout_data(loginfo->filepath, std::ios::app);
		fout_data << logstr << std::endl;
		fout_data.close();
	}

	return 0;
}



void MyLogger::mf_buildtime(STR_MYTIME *buildtime)
{

	GetLocalTime(&buildtime->mytime_st);

	SystemTimeToFileTime(&buildtime->mytime_st, &buildtime->mytime_ft_temp);

	LocalFileTimeToFileTime(&buildtime->mytime_ft_temp, &buildtime->mytime_ft_real);

	buildtime->mytime_ui.HighPart = buildtime->mytime_ft_real.dwHighDateTime;
	buildtime->mytime_ui.LowPart = buildtime->mytime_ft_real.dwLowDateTime;

	buildtime->mytime_ll = long long((buildtime->mytime_ui.QuadPart * 0.0001 - 11644473600000ULL)*0.001);


	sprintf(buildtime->mytime_str, "%d%02d%02d %02d:%02d:%02d.%03d",
		buildtime->mytime_st.wYear,
		buildtime->mytime_st.wMonth,
		buildtime->mytime_st.wDay,
		buildtime->mytime_st.wHour,
		buildtime->mytime_st.wMinute,
		buildtime->mytime_st.wSecond,
		buildtime->mytime_st.wMilliseconds
		);
	
}

