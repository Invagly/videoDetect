#include "mutexlock.h"
#include <assert.h>

namespace pacs
{
MutexLock::MutexLock()
	:_isLocking(false)
{
	TINY_CHECK(!pthread_mutex_init(&_mutex, NULL))
}

MutexLock::~MutexLock()
{
	assert(!isLocking());
	TINY_CHECK(!pthread_mutex_destroy(&_mutex))
}

void MutexLock::lock()
{
	TINY_CHECK(!pthread_mutex_lock(&_mutex))
	_isLocking = true;
}

void MutexLock::unlock()
{
	TINY_CHECK(!pthread_mutex_unlock(&_mutex))
	_isLocking = false;
}

bool MutexLock::isLocking()
{
	return _isLocking;
}

pthread_mutex_t *MutexLock::getMutexPtr()
{
	return &_mutex;
}
}