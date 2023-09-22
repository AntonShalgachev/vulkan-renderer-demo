#pragma once

#include "platform/threading.h"

namespace mt
{
    class mutex
    {
    public:
        mutex();
        ~mutex();

        void lock();
        void unlock();

    private:
        platform::mutex_storage_t m_storage;
    };
}
