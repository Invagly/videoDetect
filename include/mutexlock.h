#pragma once
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define TINY_CHECK(exp)\
	if(!(exp))\
	{\
		fprintf(stderr,"File:%s,Line:%d,Exp:[" #exp "] is true,abort.\n",__FILE__,__LINE__);abort();\
	}

namespace pacs
{
class MutexLock
{
	friend class Condition;
public:
	MutexLock();
	~MutexLock();
	void lock();
	void unlock();
	bool isLocking();
	pthread_mutex_t *getMutexPtr();
private:
	void restoreMutexStatus()
	{_isLocking = true;}
	
	pthread_mutex_t _mutex;
	bool _isLocking;
};
}