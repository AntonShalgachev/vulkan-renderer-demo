#include "platform/startup.h"

#include "windows_common.h"

#include <stdlib.h>

int run(int argc, char** argv);

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return run(__argc, __argv);
}
