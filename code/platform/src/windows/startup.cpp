#include "platform/startup.h"

#include "windows_common.h"

#include <stdlib.h>

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return run(__argc, __argv);
}
