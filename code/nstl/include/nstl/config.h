#pragma once

// Description: If this option is set, asserts would be enabled
// CMake option: NSTL_ENABLE_ASSERTS
#ifndef NSTL_CONFIG_ENABLE_ASSERTS
    #define NSTL_CONFIG_ENABLE_ASSERTS 0
#endif

#if defined(__clang__) || defined(__GNUC__)
    #define NSTL_PRINTF_LIKE(formatIndex, firstArgIndex) __attribute__((__format__(__printf__, formatIndex, firstArgIndex)))
#else
    #define NSTL_PRINTF_LIKE(formatIndex, firstArgIndex)
#endif
