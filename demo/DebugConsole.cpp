#include "DebugConsole.h"

DebugConsole& DebugConsole::instance()
{
    static DebugConsole console;
    return console;
}

void DebugConsole::execute(std::string_view command)
{
    addLine("> " + std::string{ command }, Line::Type::Input);
    m_inputHistory.push_back(std::string{ command });

    coil::ExecutionResult result = m_bindings.execute(command);
    for (std::string const& error : result.errors)
        addLine(error, Line::Type::Error);
    for (std::string line; std::getline(result.output, line); )
        addLine(line, Line::Type::Output);
    if (result.returnValue)
        addLine("Command returned '" + *result.returnValue + "'", Line::Type::ReturnValue);
}

void DebugConsole::addLine(std::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}
