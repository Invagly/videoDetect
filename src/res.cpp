#include "res.h"

std::list<int> msgList;
pacs::MutexLock  m_lock;
pacs::Condition  m_condition(m_lock);