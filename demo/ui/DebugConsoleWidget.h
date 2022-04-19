#pragma once

#include <array>
#include <vector>
#include <string>
#include <optional>

namespace ui
{
    class DebugConsoleWidget
    {
    public:
        DebugConsoleWidget();

        void draw();

        void toggle() { m_visible = !m_visible; }

    private:
        enum class HistoryDirection { Up, Down };

        void onInputChanged(std::size_t length);
        std::string const* onInputHistory(HistoryDirection direction);
        void onInputCompletion();
        void onInputSubmitted();

        void clearInput();

        std::string_view getCurrentInput() const;

    private:
        bool m_visible = false;

        std::array<char, 256> m_inputBuffer = {};
        std::size_t m_inputLength = 0;

        bool m_scrollToLast = false;

        std::optional<std::size_t> m_historyIndex;
    };
}
