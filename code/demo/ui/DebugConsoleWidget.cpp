#include "DebugConsoleWidget.h"

#include "imgui.h"
#include "magic_enum.hpp"

#include <string>
#include <string_view>
#include <set>
#include <algorithm>

#include "services/DebugConsole.h"

namespace
{
    std::size_t outputLinesCount = 15;
    std::size_t suggestionsLinesCountMax = 5;

    std::size_t clamp(std::size_t value, std::size_t from, std::size_t to)
    {
        if (value > to)
            return to;
        if (value < from)
            return from;
        return value;
    };

    auto getLineColor(DebugConsole::Line::Type type)
    {
        switch (type)
        {
        case DebugConsole::Line::Type::Input:
            return IM_COL32(127, 127, 127, 255);
        case DebugConsole::Line::Type::ReturnValue:
            return IM_COL32(0, 255, 0, 255);
        case DebugConsole::Line::Type::Output:
            return IM_COL32(255, 255, 255, 255);
        case DebugConsole::Line::Type::Error:
            return IM_COL32(255, 0, 0, 255);
		case DebugConsole::Line::Type::CommandHelp:
			return IM_COL32(255, 127, 0, 255);
        }

        return IM_COL32(255, 255, 255, 255);
    }

    auto selectedSuggestionColor = IM_COL32(255, 255, 0, 255);
    auto suggestionDescriptionColor = IM_COL32(127, 127, 127, 255);
    auto commandHelpColor = IM_COL32(127, 127, 127, 255);
}

ui::DebugConsoleWidget::DebugConsoleWidget()
{
    m_commands["ui.console.toggle"] = coil::bind(&DebugConsoleWidget::toggle, this);
}

void ui::DebugConsoleWidget::draw()
{
    if (!m_visible)
        return;

    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize({ viewport->WorkSize.x, 0.0f });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    ImGui::Begin("DebugConsole", nullptr, windowFlags);

    drawOutput();
    drawInput();

	auto name = getInputCommandName();
    if (!name.empty() && DebugConsole::instance().getMetadata(name))
        drawCommandHelp(name);
    else
        drawSuggestions();

    ImGui::End();
}

void ui::DebugConsoleWidget::drawOutput()
{
    auto const outputHeight = ImGui::GetTextLineHeightWithSpacing() * outputLinesCount + ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("OutputArea", ImVec2(0, outputHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (DebugConsole::Line const& line : DebugConsole::instance().lines())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, getLineColor(line.type));
        ImGui::Text(line.text.c_str());
        ImGui::PopStyleColor();
    }

    if (m_scrollToLast)
        ImGui::SetScrollHereY();
    m_scrollToLast = false;

    ImGui::EndChild();
}

void ui::DebugConsoleWidget::drawInput()
{
    auto editCallback = [](ImGuiInputTextCallbackData* data)
    {
        auto console = static_cast<DebugConsoleWidget*>(data->UserData);
        if (!console)
            return 0;

        auto getView = [](ImGuiInputTextCallbackData* data) { return std::string_view{ data->Buf, static_cast<std::size_t>(data->BufTextLen) }; };

        std::string_view input = getView(data);

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            console->onInputChanged(input);

        std::optional<std::string_view> replacement;

        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
        {
            replacement = console->onInputCompletion(input);
        }

        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            if (data->EventKey == ImGuiKey_UpArrow)
                replacement = console->onInputHistory(input, -1);
            if (data->EventKey == ImGuiKey_DownArrow)
                replacement = console->onInputHistory(input, 1);
        }

        if (replacement)
        {
            std::string replacementCopy = std::string{ *replacement };
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, replacementCopy.data(), replacementCopy.data() + replacementCopy.size());
            console->onInputReplaced(getView(data));
        }

        return 0;
    };

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));

    ImGui::AlignTextToFramePadding();
    ImGui::Text("> ");
    ImGui::SameLine();

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion;
    ImVec4 frameBackgroundColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    frameBackgroundColor.w = 0.0f;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, frameBackgroundColor);

    ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - ImGui::GetCursorPosX());

    if (ImGui::InputText("Input", m_inputBuffer.data(), m_inputBuffer.size(), inputFlags, editCallback, this))
        onInputSubmitted({ m_inputBuffer.data(), m_inputLength });

    if (ImGui::IsItemDeactivatedAfterEdit())
        clearInput();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::SetKeyboardFocusHere(-1);
}

void ui::DebugConsoleWidget::drawSuggestions()
{
    if (m_suggestionsWindowStart > 0)
        ImGui::Text("  ...");

    for (std::size_t i = m_suggestionsWindowStart; i < m_suggestionsWindowEnd; i++)
    {
        bool const selected = i == getSuggestionIndex();
        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Text, selectedSuggestionColor);

        {
            std::string_view command = m_suggestions[i].suggestion.command;
            std::string_view description = m_suggestions[i].description;
            ImGui::Text("  %.*s", command.size(), command.data());
            if (!description.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, suggestionDescriptionColor);
                ImGui::SameLine();
                ImGui::Text(" // %.*s", description.size(), description.data());
                ImGui::PopStyleColor();
            }
        }

        if (selected)
            ImGui::PopStyleColor();
    }

    if (m_suggestionsWindowEnd < m_suggestions.size())
        ImGui::Text("  ...");
}

void ui::DebugConsoleWidget::drawCommandHelp(std::string_view name)
{
    std::stringstream ss;
    DebugConsole::instance().getCommandHelp(ss, name);

	ImGui::PushStyleColor(ImGuiCol_Text, commandHelpColor);
	for (std::string line; std::getline(ss, line); )
        ImGui::Text("  %s", line.c_str());
	ImGui::PopStyleColor();
}

void ui::DebugConsoleWidget::onInputChanged(std::string_view input)
{
    m_oldInput = {};
    m_replacementIndex = 0;

    m_inputLength = input.size();

    m_suggestions.clear();
    for (DebugConsole::Suggestion& candidate : DebugConsole::instance().getSuggestions(input))
    {
        Suggestion suggestion;
        suggestion.suggestion = std::move(candidate);

        if (CommandMetadata const* metadata = DebugConsole::instance().getMetadata(suggestion.suggestion.command); metadata && !metadata->description.empty())
            suggestion.description = metadata->description;

        m_suggestions.push_back(std::move(suggestion));
    }

    auto const linesCount = std::min(m_suggestions.size(), suggestionsLinesCountMax);
    m_suggestionsWindowStart = 0;
    m_suggestionsWindowEnd = linesCount;
}

void ui::DebugConsoleWidget::onInputReplaced(std::string_view input)
{
    m_inputLength = input.size();
}

std::optional<std::string_view> ui::DebugConsoleWidget::onInputHistory(std::string_view input, int delta)
{
    auto const& inputHistory = DebugConsole::instance().history();

    int minIndex = -inputHistory.size();
    int maxIndex = m_suggestions.size();
    m_replacementIndex = std::min(std::max(m_replacementIndex + delta, minIndex), maxIndex);

    if (m_replacementIndex != 0 && !m_oldInput)
        m_oldInput = std::string{ input };

    if (auto index = getSuggestionIndex())
        updateSuggestionsWindow(*index);

    if (auto index = getHistoryIndex(inputHistory.size()))
        return inputHistory[*index];
    if (auto index = getSuggestionIndex())
        return m_suggestions[*index].suggestion.command;

    if (m_oldInput)
        return *m_oldInput;

    return {};
}

std::optional<std::string_view> ui::DebugConsoleWidget::onInputCompletion(std::string_view input)
{
    return DebugConsole::instance().autoComplete(input);
}

void ui::DebugConsoleWidget::onInputSubmitted(std::string_view input)
{
    if (input.empty())
        return;

    DebugConsole::instance().execute(input);

    m_scrollToLast = true;

    clearInput();
}

std::optional<std::size_t> ui::DebugConsoleWidget::getHistoryIndex(std::size_t historySize) const
{
    return m_replacementIndex < 0 ? historySize + m_replacementIndex : std::optional<std::size_t>{};
}

std::optional<std::size_t> ui::DebugConsoleWidget::getSuggestionIndex() const
{
    return m_replacementIndex > 0 ? m_replacementIndex - 1 : std::optional<std::size_t>{};
}

void ui::DebugConsoleWidget::updateSuggestionsWindow(std::size_t selectedIndex)
{
    if (selectedIndex < m_suggestionsWindowStart)
    {
        std::size_t delta = m_suggestionsWindowStart - selectedIndex;
        m_suggestionsWindowStart -= delta;
        m_suggestionsWindowEnd -= delta;
        return;
    }

    if (selectedIndex >= m_suggestionsWindowEnd)
    {
        std::size_t delta = selectedIndex - m_suggestionsWindowEnd + 1;
        m_suggestionsWindowStart += delta;
        m_suggestionsWindowEnd += delta;
        return;
    }
}

std::string_view ui::DebugConsoleWidget::getInputCommandName() const
{
	std::string_view input{ m_inputBuffer.data(), m_inputLength };
	auto spacePos = input.find(' ');
    if (spacePos == std::string_view::npos)
        return {};

    return input.substr(0, spacePos);
}

void ui::DebugConsoleWidget::clearInput()
{
    m_inputLength = 0;
    m_inputBuffer = {};

    m_oldInput = {};
    m_replacementIndex = 0;

    onInputChanged({});
}
