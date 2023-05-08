#include "platform/startup.h"

#include "windows_common.h"

#include <stdlib.h>

int run(int argc, char** argv);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    return run(__argc, __argv);
}
