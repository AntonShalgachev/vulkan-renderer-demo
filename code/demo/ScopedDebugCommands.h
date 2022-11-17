#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"

#include "services/ServiceContainer.h"
#include "services/Services.h"
#include "services/DebugConsoleService.h"

#include "nstl/vector.h"
#include "nstl/string_view.h"

class ScopedDebugCommands : public ServiceContainer
{
public:
    ScopedDebugCommands(Services& services);
    ScopedDebugCommands(ScopedDebugCommands&& rhs);

    ~ScopedDebugCommands();

    ScopedDebugCommands& operator=(ScopedDebugCommands&& rhs);

    template<typename Functor>
    void add(nstl::string_view name, CommandMetadata metadata, Functor functor)
    {
        // TODO move it somewhere, preferably coil
        static_assert(coil::detail::FuncTraits<Functor>::isFunc, "Func should be a functor object");
        using FunctionWrapper = typename coil::detail::FuncTraits<Functor>::FunctionWrapperType;
        return add(name, nstl::move(metadata), coil::AnyFunctor{ FunctionWrapper{coil::move(functor)} });
    }
    void add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor);
    void add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors);

    void remove(nstl::string_view name);
    void clear();

    CommandProxy<ScopedDebugCommands> operator[](nstl::string_view name);

private:
    nstl::vector<nstl::string> m_names;
};

// TODO find the right way to do it
extern template class CommandProxy<ScopedDebugCommands>;
