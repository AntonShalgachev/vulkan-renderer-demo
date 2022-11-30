#include "ResourceContainer.h"

namespace vkgfx
{
    void testResourceContainer()
    {
        struct Data
        {
            std::uint64_t value = 0;
        };

        vkgfx::ResourceContainer<Data> container;

        vkgfx::ResourceHandle h0;
        assert(container.get(h0) == nullptr);

        vkgfx::ResourceHandle h1 = container.add(Data{ 1 });
        vkgfx::ResourceHandle h2 = container.add(Data{ 2 });
        vkgfx::ResourceHandle h3 = container.add(Data{ 3 });
        vkgfx::ResourceHandle h4 = container.add(Data{ 4 });

        assert(container.get(h0) == nullptr);
        assert(container.get(h1)->value == 1);
        assert(container.get(h2)->value == 2);
        assert(container.get(h3)->value == 3);
        assert(container.get(h4)->value == 4);

        container.remove(h2);

        assert(container.get(h0) == nullptr);
        assert(container.get(h1)->value == 1);
        assert(container.get(h2) == nullptr);
        assert(container.get(h3)->value == 3);
        assert(container.get(h4)->value == 4);

        container.remove(h1);

        assert(container.get(h0) == nullptr);
        assert(container.get(h1) == nullptr);
        assert(container.get(h2) == nullptr);
        assert(container.get(h3)->value == 3);
        assert(container.get(h4)->value == 4);

        vkgfx::ResourceHandle h5 = container.add(Data{ 5 });
        vkgfx::ResourceHandle h6 = container.add(Data{ 6 });

        assert(container.get(h0) == nullptr);
        assert(container.get(h1) == nullptr);
        assert(container.get(h2) == nullptr);
        assert(container.get(h3)->value == 3);
        assert(container.get(h4)->value == 4);
        assert(container.get(h5)->value == 5);
        assert(container.get(h6)->value == 6);
    }
}

size_t nstl::hash<vkgfx::ResourceHandle>::operator()(vkgfx::ResourceHandle const& value) const
{
    return nstl::hash_values(value.index, value.reincarnation);
}
