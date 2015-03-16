#ifndef _CLOG_H
#define _CLOG_H
#pragma once

#include <fstream>
#include <ctime>

class CLog
{
public:
	~CLog()
	{
		//_instance = 0;
		if (pf_)
		{
			pf_->close();
			delete pf_;
		}
	}

	static CLog* GetPtr()
	{
		if (!_instance)
			_instance = new CLog;
		return(_instance);
	}

	static CLog& GetObj()
	{
		if (!_instance)
			_instance = new CLog;
		return (*_instance);
	}

	template<class T> inline CLog& Write(T val)
	{
		(*pf_) << val;
		pf_->flush();
		return *this;
	}

	template<class T> inline CLog& tmWrite(T val)
	{
		char strBuf[50];
		time_t ltime;

		time(&ltime);
		struct tm today;
		localtime_s(&today, &ltime);
		sprintf_s(strBuf, sizeof(strBuf), "[M-%2.2d-%2.2d %2.2d:%2.2d:%2.2d] ",
			today.tm_year + 1900, today.tm_mon + 1, today.tm_mday, today.tm_hour,
			today.tm_min, today.tm_sec);

		(*pf_) << strBuf << val << "\r\n";
		pf_->flush();
		return *this;
	}

	template<class T> inline CLog& operator<< (T val)
	{
		(*pf_) << val;
		pf_->flush();
		return *this;
	}

private:
	CLog()
	{
		pf_ = new std::ofstream("clog.log", std::ios::app);
	}

	std::ofstream* pf_;

	static CLog* _instance;

};


#endif