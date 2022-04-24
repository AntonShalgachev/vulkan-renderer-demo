#include "DebugConsoleWidget.h"

#include "imgui.h"
#include "magic_enum.hpp"

#include <string>
#include <string_view>
#include "../DebugConsole.h"

namespace
{
    float heightPercentage = 0.3f;

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
        }

        return IM_COL32(255, 255, 255, 255);
    }
}

ui::DebugConsoleWidget::DebugConsoleWidget()
{
    coil::Bindings& bindings = DebugConsole::instance().bindings();

    bindings["ui.console.toggle"] = coil::bind(&DebugConsoleWidget::toggle, this);
}

void ui::DebugConsoleWidget::draw()
{
    if (!m_visible)
        return;

    DebugConsole& console = DebugConsole::instance();

    auto viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize({ workSize.x, workSize.y * heightPercentage });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    ImGui::Begin("DebugConsole", nullptr, windowFlags);

    auto const inputHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -inputHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (DebugConsole::Line const& line : console.lines())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, getLineColor(line.type));
        ImGui::Text(line.text.c_str());
        ImGui::PopStyleColor();
    }

    if (m_scrollToLast)
        ImGui::SetScrollHereY();
    m_scrollToLast = false;

    ImGui::EndChild();

    auto editCallback = [](ImGuiInputTextCallbackData* data) {
        auto console = static_cast<DebugConsoleWidget*>(data->UserData);
        if (!console)
            return 0;

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            console->onInputChanged(data->BufTextLen);

        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
            console->onInputCompletion();

        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            std::string const* replacement = nullptr;

            if (data->EventKey == ImGuiKey_UpArrow)
                replacement = console->onInputHistory(HistoryDirection::Up);
            if (data->EventKey == ImGuiKey_DownArrow)
                replacement = console->onInputHistory(HistoryDirection::Down);

            if (replacement)
            {
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, replacement->c_str());
                console->onInputChanged(data->BufTextLen);
            }
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

    bool shouldFocus = ImGui::IsWindowAppearing();
    if (ImGui::InputText("Input", m_inputBuffer.data(), m_inputBuffer.size(), inputFlags, editCallback, this))
    {
        onInputSubmitted();
        shouldFocus = true;
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    if (shouldFocus)
        ImGui::SetKeyboardFocusHere(-1);

    ImGui::End();
}

std::string_view ui::DebugConsoleWidget::getCurrentInput() const
{
    return std::string_view(m_inputBuffer.data(), m_inputLength);
}

void ui::DebugConsoleWidget::onInputChanged(std::size_t length)
{
    m_inputLength = length;
}

std::string const* ui::DebugConsoleWidget::onInputHistory(HistoryDirection direction)
{
    auto const& inputHistory = DebugConsole::instance().history();

    auto computeNewIndex = [this, direction, &inputHistory]() -> std::optional<std::size_t>
    {
        if (inputHistory.size() == 0)
            return {};

        switch (direction)
        {
        case HistoryDirection::Up:
            if (!m_historyIndex)
                return inputHistory.size() - 1;
            if (*m_historyIndex == 0)
                return m_historyIndex;
            return clamp(*m_historyIndex - 1, 0, inputHistory.size() - 1);
        case HistoryDirection::Down:
            return clamp(*m_historyIndex + 1, 0, inputHistory.size() - 1);
        }

        return {};
    };

    m_historyIndex = computeNewIndex();

    if (!m_historyIndex)
        return nullptr;

    return &inputHistory[*m_historyIndex];
}

void ui::DebugConsoleWidget::onInputCompletion()
{

}

void ui::DebugConsoleWidget::onInputSubmitted()
{
    std::string_view input = getCurrentInput();

    if (input.empty())
        return;

    DebugConsole::instance().execute(input);

    m_scrollToLast = true;

    clearInput();
}

void ui::DebugConsoleWidget::clearInput()
{
    m_inputLength = 0;
    m_inputBuffer = {};
    m_historyIndex = {};
}
