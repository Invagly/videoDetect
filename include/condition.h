#pragma once
#include <pthread.h>

namespace pacs
{
class MutexLock;
class Condition
{
public:
	Condition(MutexLock &mutex);
	~Condition();

	void wait();
	void notify();

private:
	pthread_cond_t _cond;
	MutexLock &_mutex;
};
}