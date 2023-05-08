#include "DemoApplication.h"

int run(int argc, char** argv)
{
    DemoApplication app;

    if (!app.init(argc, argv))
        return EXIT_FAILURE;

    app.run();

    return EXIT_SUCCESS;
}
