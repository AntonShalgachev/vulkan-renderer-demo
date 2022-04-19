#pragma once

#include <array>
#include <vector>
#include <string>
#include <optional>

namespace ui
{
    class DebugConsole
    {
    public:
        void draw();

    private:
        enum class HistoryDirection { Up, Down };

        void addLine(std::string line);

        void onInputChanged(std::size_t length);
        std::string* onInputHistory(HistoryDirection direction);
        void onInputCompletion();
        void onInputSubmitted();

        void clearInput();

        std::string_view getCurrentInput() const;

        std::array<char, 256> m_inputBuffer = {};
        std::size_t m_inputLength = 0;

        std::vector<std::string> m_lines;
        bool m_scrollToLast = false;

        std::vector<std::string> m_inputHistory;
        std::optional<std::size_t> m_historyIndex;
    };
}