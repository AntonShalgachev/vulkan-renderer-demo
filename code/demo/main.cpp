#include "DemoApplication.h"
#include "services/CommandLine.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <filesystem>

int main(int argc, char** argv)
{
    CommandLine& commandLine = CommandLine::instance();
    DemoApplication::registerCommandLineOptions(commandLine); // TODO move somewhere to allow others to register custom options

    spdlog::info("Current directory: {}", std::filesystem::current_path());

    if (!std::filesystem::exists("data"))
        spdlog::warn("Current directory doesn't contain 'data', probably wrong directory");

    if (!commandLine.parse(argc, argv))
	{
        spdlog::critical("Failed to parse command line arguments");
        return EXIT_FAILURE;
    }
    if (!commandLine.parseFile("data/cmdline.ini"))
	{
		spdlog::critical("Failed to parse cmdline.ini");
        return EXIT_FAILURE;
    }

    {
        std::stringstream ss;
        for (std::string const& argument : commandLine.getAll())
            ss << "'" << argument << "' ";

        spdlog::info("Command line arguments: {}", ss.str());
    }

    DemoApplication app;
    app.run();

    return EXIT_SUCCESS;
}
