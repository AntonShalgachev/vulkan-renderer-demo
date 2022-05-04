#include "DemoApplication.h"
#include "CommandLine.h"

int main(int argc, char** argv)
{
    CommandLine& commandLine = CommandLine::instance();

    DemoApplication::registerCommandLineOptions(commandLine); // TODO move somewhere to allow others to register custom options

    if (!commandLine.parse(argc, argv))
        return EXIT_FAILURE;
    if (!commandLine.parseFile("data/cmdline.ini"))
        return EXIT_FAILURE;

    DemoApplication app;
    app.run();

    return EXIT_SUCCESS;
}
