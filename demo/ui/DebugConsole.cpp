#include "DebugConsole.h"

#include "imgui.h"
#include "magic_enum.hpp"

#include <string>
#include <string_view>

namespace
{
    float heightPercentage = 0.3f;
}

void ui::DebugConsole::draw()
{
    ImGui::ShowDemoWindow();

    auto viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize({ workSize.x, workSize.y * heightPercentage });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    ImGui::Begin("DebugConsole", nullptr, windowFlags);

    auto const inputHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -inputHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (std::string const& line : m_lines)
        ImGui::Text(line.c_str());

    if (m_scrollToLast)
        ImGui::SetScrollHereY();
    m_scrollToLast = false;

    ImGui::EndChild();

    auto editCallback = [](ImGuiInputTextCallbackData* data) {
        auto console = static_cast<DebugConsole*>(data->UserData);
        if (!console)
            return 0;

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            console->onInputChanged(data->BufTextLen);

        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
            console->onInputCompletion();

        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            if (data->EventKey == ImGuiKey_UpArrow)
                console->onInputHistory(HistoryDirection::Up);
            if (data->EventKey == ImGuiKey_DownArrow)
                console->onInputHistory(HistoryDirection::Down);
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

std::string_view ui::DebugConsole::getCurrentInput() const
{
    return std::string_view(m_inputBuffer.data(), m_inputLength);
}

void ui::DebugConsole::addLine(std::string line)
{
    m_lines.push_back(std::move(line));
    m_scrollToLast = true;
}

void ui::DebugConsole::onInputChanged(std::size_t length)
{
    m_inputLength = length;
    addLine("Input changed");
}

void ui::DebugConsole::onInputHistory(HistoryDirection direction)
{
    addLine(std::string("Input history ") + std::string(magic_enum::enum_name(direction)));
}

void ui::DebugConsole::onInputCompletion()
{
    addLine("Input completion");
}

void ui::DebugConsole::onInputSubmitted()
{
    addLine("> " + std::string(getCurrentInput()));

    clearInput();
}

void ui::DebugConsole::clearInput()
{
    m_inputLength = 0;
    m_inputBuffer = {};
}
