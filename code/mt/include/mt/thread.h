#pragma once

#include "platform/threading.h"

namespace mt
{
    class thread
    {
    public:
        template<typename F, typename... T>
        thread(F&& f, T&&... args) : thread()
        {
            // TODO implement
        }

        thread();

        void join();

    private:
        platform::thread_storage_t m_storage;
    };
}
