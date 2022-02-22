#pragma once

#include <array>
#include <vector>
#include <string>

namespace ui
{
    class DebugConsole
    {
    public:
        void draw();

    private:
        void addLine(std::string line);
        void onInputChanged(std::size_t length);

        std::string_view getCurrentInput() const;

        std::array<char, 256> m_inputBuffer = {};
        std::size_t m_inputLength = 0;
        bool m_inputChanged = false;

        std::vector<std::string> m_lines;
        bool m_linesChanged = false;
    };
}
