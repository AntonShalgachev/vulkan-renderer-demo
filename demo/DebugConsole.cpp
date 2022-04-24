#include "DebugConsole.h"

#include <iomanip>

DebugConsole& DebugConsole::instance()
{
    static DebugConsole console;
    return console;
}

DebugConsole::DebugConsole()
{
    m_bindings["list"] = [this](coil::Context context)
    {
        context.out() << "Available commands:" << std::endl;
        for (auto const& command : m_bindings.commands())
            context.out() << command << std::endl;
    };

    m_bindings["help"] = [](coil::Context context)
    {
        context.out() << std::setw(15) << std::left << "Command name" << '\t' << "Description" << std::endl;
        context.out() << std::setw(15) << std::left << "list" << '\t' << "Lists available commands" << std::endl;
        context.out() << std::setw(15) << std::left << "help" << '\t' << "Prints this message" << std::endl;
    };
}

void DebugConsole::execute(std::string_view command)
{
    addLine("> " + std::string{ command }, Line::Type::Input);
    m_inputHistory.push_back(std::string{ command });

    coil::ExecutionResult result = m_bindings.execute(command);
    for (std::string const& error : result.errors)
        addLine("  " + error, Line::Type::Error);
    for (std::string line; std::getline(result.output, line); )
        addLine("  " + line, Line::Type::Output);
    if (result.returnValue)
        addLine("  -> '" + *result.returnValue + "'", Line::Type::ReturnValue);
}

void DebugConsole::addLine(std::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}
