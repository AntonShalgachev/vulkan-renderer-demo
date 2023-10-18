#include "platform/startup.h"

#include "common.h"

#include <stdlib.h>

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return run(__argc, __argv);
}
