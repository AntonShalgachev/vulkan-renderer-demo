#pragma once

#include "picofmt/detail/writer_base.h"
#include "picofmt/detail/simple_string_view.h"

namespace picofmt
{
    class args_list;
    class context;

    namespace detail
    {
        class context_base
        {
        public:
            context_base(writer_base& writer_base, args_list const& args) : writer_base(writer_base), args(args) {}

            bool write(simple_string_view str) const { return writer_base.write(str); }
            bool write(char c, size_t count) const { return writer_base.write(c, count); }
            void report_error(simple_string_view str) const { return writer_base.report_error(str); }

            size_t get_next_arg_id() const { return m_next_arg_id; }
            void consume_arg(size_t id) { m_next_arg_id = id + 1; }

            virtual context& get_user_context() = 0;

            writer_base& writer_base;
            args_list const& args;

        private:
            size_t m_next_arg_id = 0;
        };
    }
}
