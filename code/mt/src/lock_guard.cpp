#include "mt/lock_guard.h"

#include "mt/mutex.h"

mt::lock_guard::lock_guard(mt::mutex& mutex) : m_mutex(mutex)
{
    m_mutex.lock();
}

mt::lock_guard::~lock_guard()
{
    m_mutex.unlock();
}
