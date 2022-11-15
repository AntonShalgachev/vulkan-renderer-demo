#pragma once

#include "Config.h"

#if NSTL_CONFIG_ENABLE_ASSERTS

    #include <assert.h>
    #define NSTL_ASSERT(...) assert(__VA_ARGS__)

#else

    #define NSTL_ASSERT(...) ((void)0)

#endif
