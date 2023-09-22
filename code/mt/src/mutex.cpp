#include "mt/mutex.h"

#include <assert.h>

mt::mutex::mutex()
{
    bool result = platform::mutex_create(m_storage);
    assert(result);
}

mt::mutex::~mutex()
{
    platform::mutex_destroy(m_storage);
}

void mt::mutex::lock()
{
    platform::mutex_lock(m_storage);
}

void mt::mutex::unlock()
{
    platform::mutex_unlock(m_storage);
}
