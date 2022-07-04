#pragma once

#include <array>
#include <vector>
#include <string>
#include <optional>

#include "../ScopedDebugCommands.h"

namespace ui
{
    class DebugConsoleWidget
    {
    public:
        DebugConsoleWidget();

        void draw();

        void toggle() { m_visible = !m_visible; }

    private:
        void drawOutput();
        void drawInput();
        void drawSuggestions();
        void drawCommandHelp(std::string_view name);

        void onInputChanged(std::string_view input);
        void onInputReplaced(std::string_view input);
        std::optional<std::string_view> onInputHistory(std::string_view input, int delta);
        std::optional<std::string_view> onInputCompletion(std::string_view input);
        void onInputSubmitted(std::string_view input);

        std::optional<std::size_t> getHistoryIndex(std::size_t historySize) const;
        std::optional<std::size_t> getSuggestionIndex() const;

        void updateSuggestionsWindow(std::size_t selectedIndex);

        std::string_view getInputCommandName() const;

        void clearInput();

    private:
        struct Suggestion
        {
            DebugConsole::Suggestion suggestion;
            std::string_view description;
        };

        ScopedDebugCommands m_commands;

        bool m_visible = false;

        std::array<char, 256> m_inputBuffer = {};
        std::size_t m_inputLength = 0;

        bool m_scrollToLast = false;

        std::optional<std::string> m_oldInput;
        int m_replacementIndex = 0;

        std::vector<Suggestion> m_suggestions;
        std::size_t m_suggestionsWindowStart = 0;
        std::size_t m_suggestionsWindowEnd = 0;
    };
}
