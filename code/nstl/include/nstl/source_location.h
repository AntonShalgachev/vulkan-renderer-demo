#pragma once

namespace nstl
{
    struct source_location
    {
        [[nodiscard]] constexpr source_location(int line = __builtin_LINE(), int column = __builtin_COLUMN(), char const* file = __builtin_FILE(), char const* function = __builtin_FUNCTION())
            : m_line(line)
            , m_column(column)
            , m_file(file)
            , m_function(function)
        {

        }

        [[nodiscard]] constexpr int line() const noexcept {
            return m_line;
        }
        [[nodiscard]] constexpr int column() const noexcept {
            return m_column;
        }
        [[nodiscard]] constexpr const char* file_name() const noexcept {
            return m_file;
        }
        [[nodiscard]] constexpr const char* function_name() const noexcept {
            return m_function;
        }

    private:
        int m_line = -1;
        int m_column = -1;
        const char* m_file = "";
        const char* m_function = "";
    };
}
