#include "DemoApplication.h"

#include "platform/startup.h"

#include "stdlib.h"

int run(int argc, char** argv)
{
    DemoApplication app;

    if (!app.init(argc, argv))
        return EXIT_FAILURE;

    app.run();

    return EXIT_SUCCESS;
}
