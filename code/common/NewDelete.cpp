#include <stdint.h>
#include <assert.h>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}
void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    assert(false);
    return new uint8_t[size];
}