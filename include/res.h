#pragma once
#include <string>
#include <time.h>
#include <list>
#include "mutexlock.h"
#include "condition.h"
const std::string LOGNAME = "log";

namespace pacs
{
	inline int getTimeStamp()
	{
		return time((time_t*)NULL);
	}
}

extern std::list<int> msgList;
extern pacs::MutexLock  m_lock;
extern pacs::Condition  m_condition;
