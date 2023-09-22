#pragma once

namespace mt
{
    class mutex;

    class lock_guard
    {
    public:
        lock_guard(mt::mutex& mutex);
        ~lock_guard();

    private:
        mt::mutex& m_mutex;
    };
}
