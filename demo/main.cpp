#include "DemoApplication.h"
#include "CommandLine.h"

int main(int argc, char** argv)
{
    {
        CommandLine& commandLine = CommandLine::instance();

        DemoApplication app;
        app.setupCommandLine(commandLine);

        if (!commandLine.parse(argc, argv))
            return EXIT_FAILURE;
        if (!commandLine.parseFile("data/cmdline.ini"))
            return EXIT_FAILURE;

        app.run();
    }

    // temporary to catch Vulkan errors
    std::getchar();
    return EXIT_SUCCESS;
}
