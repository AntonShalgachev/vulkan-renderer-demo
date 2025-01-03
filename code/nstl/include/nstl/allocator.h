#pragma once

#include "assert.h"
#include "type_traits.h"
#include "new.h"
#include "utility.h"

#include <stddef.h>

namespace nstl
{
    template<typename T>
    concept allocator_like = requires(T& allocator, size_t size, size_t alignment, void* ptr, T const& lhs, T const& rhs)
    {
        { allocator.allocate(size, alignment) } -> same_as<void*>;
        { allocator.deallocate(ptr) };
        { lhs == rhs} -> same_as<bool>;
    };

    // TODO store small allocators on the stack (make the size configurable, like any_allocator<64>)
    class any_allocator
    {
    public:
        any_allocator() = default;

        template<allocator_like Alloc>
        any_allocator(Alloc allocator)
        {
            m_allocator = allocator.allocate(sizeof(Alloc), alignof(Alloc));
            new(nstl::new_tag{}, m_allocator) Alloc(nstl::move(allocator));

            m_destructStorage = [](any_allocator* self)
            {
                NSTL_ASSERT(self->m_allocator != nullptr);

                Alloc* allocator = static_cast<Alloc*>(self->m_allocator);

                // 1. move allocator to a local variable
                Alloc local_allocator = nstl::move(*allocator);

                // 2. destruct the allocator on the heap
                allocator->~Alloc();

                // 3. deallocate memory for the allocator
                local_allocator.deallocate(self->m_allocator);
                self->m_allocator = nullptr;
            };

            m_compare = [](any_allocator const& lhs, any_allocator const& rhs)
            {
                NSTL_ASSERT(lhs.m_allocator != nullptr);
                NSTL_ASSERT(rhs.m_allocator != nullptr);

                return *static_cast<Alloc const*>(lhs.m_allocator) == *static_cast<Alloc const*>(rhs.m_allocator);
            };

            m_copy = [](any_allocator const* source, any_allocator* destination)
            {
                NSTL_ASSERT(source->m_allocator != nullptr);
                NSTL_ASSERT(destination->m_allocator == nullptr);

                Alloc* allocator = static_cast<Alloc*>(source->m_allocator);
                destination->m_allocator = allocator->allocate(sizeof(Alloc), alignof(Alloc));
                new(nstl::new_tag{}, destination->m_allocator) Alloc(*allocator);
            };

            m_allocate = [](void* allocator, size_t size, size_t alignment) { return static_cast<Alloc*>(allocator)->allocate(size, alignment); };
            m_deallocate = [](void* allocator, void* ptr) { return static_cast<Alloc*>(allocator)->deallocate(ptr); };
        }

        any_allocator(any_allocator const&);
        any_allocator(any_allocator&& rhs);

        ~any_allocator();

        any_allocator& operator=(any_allocator const&);
        any_allocator& operator=(any_allocator&&);

        void* allocate(size_t size, size_t alignment);
        void deallocate(void* ptr);

        explicit operator bool() const;

        bool operator==(any_allocator const& rhs) const;

    private:
        static void swap(any_allocator& lhs, any_allocator& rhs);

    private:
        using CopyFunc = void(*)(any_allocator const* source, any_allocator* destination);
        using CompareFunc = bool(*)(any_allocator const& lhs, any_allocator const& rhs);
        using DestructStorageFunc = void(*)(any_allocator*);

        using AllocFunc = void* (*)(void* allocator, size_t size, size_t alignment);
        using DeallocFunc = void(*)(void* allocator, void* ptr);

        void* m_allocator = nullptr;
        DestructStorageFunc m_destructStorage = nullptr;
        CompareFunc m_compare = nullptr;
        CopyFunc m_copy = nullptr;

        AllocFunc m_allocate = nullptr;
        DeallocFunc m_deallocate = nullptr;
    };
}
