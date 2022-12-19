#include "DebugConsoleWidget.h"

#include "services/DebugConsoleService.h"

#include "common/Utils.h"

#include "imgui.h"

#include "nstl/algorithm.h"
#include "nstl/string_builder.h"

namespace
{
    std::size_t outputLinesCount = 15;
    std::size_t suggestionsLinesCountMax = 5;

    auto getLineColor(DebugConsoleService::Line::Type type)
    {
        switch (type)
        {
        case DebugConsoleService::Line::Type::Input:
            return IM_COL32(127, 127, 127, 255);
        case DebugConsoleService::Line::Type::ReturnValue:
            return IM_COL32(0, 255, 0, 255);
        case DebugConsoleService::Line::Type::Output:
            return IM_COL32(255, 255, 255, 255);
        case DebugConsoleService::Line::Type::Error:
            return IM_COL32(255, 0, 0, 255);
		case DebugConsoleService::Line::Type::CommandHelp:
			return IM_COL32(255, 127, 0, 255);
        }

        return IM_COL32(255, 255, 255, 255);
    }

    auto selectedSuggestionColor = IM_COL32(255, 255, 0, 255);
    auto suggestionDescriptionColor = IM_COL32(127, 127, 127, 255);
    auto commandHelpColor = IM_COL32(127, 127, 127, 255);
}

ui::DebugConsoleWidget::DebugConsoleWidget(Services& services) : ServiceContainer(services)
{
    // TODO fix: this is captured, but the object is later relocated
//     m_commands["ui.console.toggle"] = coil::bind(&DebugConsoleWidget::toggle, this);
//     m_commands["ui.console.show-score"] = coil::variable(&m_showScore);
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
    if (!name.empty() && services().debugConsole().getMetadata(name))
        drawCommandHelp(name);
    else
        drawSuggestions();

    ImGui::End();
}

void ui::DebugConsoleWidget::drawOutput()
{
    auto const outputHeight = ImGui::GetTextLineHeightWithSpacing() * outputLinesCount + ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("OutputArea", ImVec2(0, outputHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (DebugConsoleService::Line const& line : services().debugConsole().lines())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, getLineColor(line.type));
        ImGui::Text("%s", line.text.c_str());
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

        auto getView = [](ImGuiInputTextCallbackData* data) { return nstl::string_view{ data->Buf, static_cast<std::size_t>(data->BufTextLen) }; };

        nstl::string_view input = getView(data);

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            console->onInputChanged(input);

        nstl::optional<nstl::string_view> replacement;

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
            nstl::string replacementCopy = nstl::string{ *replacement };
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
            nstl::string_view command = m_suggestions[i].suggestion.command;
            size_t distance = m_suggestions[i].suggestion.distance;
            nstl::string_view description = m_suggestions[i].description;

            if (m_showScore)
                ImGui::Text("  [%zu] %.*s", distance, command.slength(), command.data());
            else
                ImGui::Text("  %.*s", command.slength(), command.data());

            if (!description.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, suggestionDescriptionColor);
                ImGui::SameLine();
                ImGui::Text(" // %.*s", description.slength(), description.data());
                ImGui::PopStyleColor();
            }
        }

        if (selected)
            ImGui::PopStyleColor();
    }

    if (m_suggestionsWindowEnd < m_suggestions.size())
        ImGui::Text("  ...");
}

void ui::DebugConsoleWidget::drawCommandHelp(nstl::string_view name)
{
    nstl::string_builder builder;
    services().debugConsole().getCommandHelp(builder, name);
    nstl::string commandHelp = builder.build();

	ImGui::PushStyleColor(ImGuiCol_Text, commandHelpColor);
    for (nstl::string_view line : vkc::utils::split(commandHelp))
        ImGui::Text("  %.*s", line.slength(), line.data());
	ImGui::PopStyleColor();
}

void ui::DebugConsoleWidget::onInputChanged(nstl::string_view input)
{
    m_oldInput = {};
    m_replacementIndex = 0;

    m_inputLength = input.size();

    m_suggestions.clear();
    for (DebugConsoleService::Suggestion& candidate : services().debugConsole().getSuggestions(input))
    {
        Suggestion suggestion;
        suggestion.suggestion = std::move(candidate);

        if (CommandMetadata const* metadata = services().debugConsole().getMetadata(suggestion.suggestion.command); metadata && !metadata->description.empty())
            suggestion.description = metadata->description;

        m_suggestions.push_back(std::move(suggestion));
    }

    auto const linesCount = nstl::min(m_suggestions.size(), suggestionsLinesCountMax);
    m_suggestionsWindowStart = 0;
    m_suggestionsWindowEnd = linesCount;
}

void ui::DebugConsoleWidget::onInputReplaced(nstl::string_view input)
{
    m_inputLength = input.size();
}

nstl::optional<nstl::string_view> ui::DebugConsoleWidget::onInputHistory(nstl::string_view input, int delta)
{
    auto const& inputHistory = services().debugConsole().history();

    int minIndex = -inputHistory.size();
    int maxIndex = m_suggestions.size();
    m_replacementIndex = nstl::clamp(m_replacementIndex + delta, minIndex, maxIndex);

    if (m_replacementIndex != 0 && !m_oldInput)
        m_oldInput = nstl::string{ input };

    if (auto index = getSuggestionIndex())
        updateSuggestionsWindow(*index);

    if (auto index = getHistoryIndex(inputHistory.size()))
        return nstl::string_view{ inputHistory[*index] };
    if (auto index = getSuggestionIndex())
        return m_suggestions[*index].suggestion.command;

    if (m_oldInput)
        return nstl::string_view{ *m_oldInput };

    return {};
}

nstl::optional<nstl::string_view> ui::DebugConsoleWidget::onInputCompletion(nstl::string_view input)
{
    return services().debugConsole().autoComplete(input);
}

void ui::DebugConsoleWidget::onInputSubmitted(nstl::string_view input)
{
    if (input.empty())
        return;

    services().debugConsole().execute(input);

    m_scrollToLast = true;

    clearInput();
}

nstl::optional<std::size_t> ui::DebugConsoleWidget::getHistoryIndex(std::size_t historySize) const
{
    return m_replacementIndex < 0 ? historySize + m_replacementIndex : nstl::optional<std::size_t>{};
}

nstl::optional<std::size_t> ui::DebugConsoleWidget::getSuggestionIndex() const
{
    return m_replacementIndex > 0 ? static_cast<std::size_t>(m_replacementIndex - 1) : nstl::optional<std::size_t>{};
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

nstl::string_view ui::DebugConsoleWidget::getInputCommandName() const
{
	nstl::string_view input{ m_inputBuffer.data(), m_inputLength };

    if (auto pos = input.find(' '); pos != nstl::string_view::npos)
        return input.substr(0, pos);

	return {};
}

void ui::DebugConsoleWidget::clearInput()
{
    m_inputLength = 0;
    m_inputBuffer = {};

    m_oldInput = {};
    m_replacementIndex = 0;

    onInputChanged({});
}
