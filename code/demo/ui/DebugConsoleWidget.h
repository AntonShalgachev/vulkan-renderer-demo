#pragma once

#include "services/ServiceContainer.h"

#include "ScopedDebugCommands.h"

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/vector.h"
#include "nstl/optional.h"
#include "nstl/array.h"

namespace ui
{
    class DebugConsoleWidget : public ServiceContainer
    {
    public:
        DebugConsoleWidget(Services& services);

        void draw();

        void toggle() { m_visible = !m_visible; }

    private:
        void drawOutput();
        void drawInput();
        void drawSuggestions();
        void drawCommandHelp(nstl::string_view name);

        void onInputChanged(nstl::string_view input);
        void onInputReplaced(nstl::string_view input);
        nstl::optional<nstl::string_view> onInputHistory(nstl::string_view input, int delta);
        nstl::optional<nstl::string_view> onInputCompletion(nstl::string_view input);
        void onInputSubmitted(nstl::string_view input);

        nstl::optional<size_t> getHistoryIndex(size_t historySize) const;
        nstl::optional<size_t> getSuggestionIndex() const;

        void updateSuggestionsWindow(size_t selectedIndex);

        nstl::string_view getInputCommandName() const;

        void clearInput();

    private:
        struct Suggestion
        {
            DebugConsoleService::Suggestion suggestion;
            nstl::string_view description;
        };

        ScopedDebugCommands m_commands{ services() };

        bool m_visible = false;
        bool m_showScore = false;

        nstl::array<char, 256> m_inputBuffer = {};
        size_t m_inputLength = 0;

        bool m_scrollToLast = false;

        nstl::optional<nstl::string> m_oldInput;
        int m_replacementIndex = 0;

        nstl::vector<Suggestion> m_suggestions;
        size_t m_suggestionsWindowStart = 0;
        size_t m_suggestionsWindowEnd = 0;
    };
}
