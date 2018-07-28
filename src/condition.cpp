#include "condition.h"
#include "mutexlock.h"
#include <assert.h>

namespace pacs
{
Condition::Condition(MutexLock& mutex)
	:_mutex(mutex)
{
	TINY_CHECK(!pthread_cond_init(&_cond,NULL));
}

Condition::~Condition()
{
	TINY_CHECK(!pthread_cond_destroy(&_cond));
}

void Condition::wait()
{
	assert(_mutex.isLocking());
	TINY_CHECK(!pthread_cond_wait(&_cond, _mutex.getMutexPtr()));
	_mutex.restoreMutexStatus();             //what does this line mean???
}

void Condition::notify()
{
	TINY_CHECK(!pthread_cond_signal(&_cond));
}
}