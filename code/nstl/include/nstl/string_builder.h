#pragma once

#include "nstl/string.h"
#include "nstl/string_view.h"
#include "nstl/vector.h"

namespace nstl
{
    class string_builder
    {
    public:
        string_builder& append(string_view str);
        string_builder& append(char c);
        string build() const;

    private:
        vector<string> m_chunks;
    };
}
