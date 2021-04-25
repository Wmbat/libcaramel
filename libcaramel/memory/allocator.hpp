#pragma once

#include <cstddef>

#include <libcaramel/memory/memory_resource.hpp>
#include <libcaramel/util/crtp.hpp>
#include <libcaramel/util/types.hpp>

#include <gsl/pointers>

namespace caramel
{
   template <typename Any>
   class allocator
   {
   public:
      using pointer = Any*;
      using const_pointer = const Any*;

   public:
      allocator() noexcept : mp_resource(get_default_memory_resource()) {}
      allocator(memory_resource* p_resource) noexcept : mp_resource{p_resource} {}
      template <typename U>
      allocator(const allocator<U>& other) noexcept : mp_resource{other.resource()}
      {}

      auto operator==(const allocator& alloc) const -> bool
      {
         return *resource() == *alloc.resource();
      }

      auto allocate(count_t count) -> pointer
      {
         return static_cast<pointer>(
            mp_resource->allocate(count_t{sizeof(Any)} * count, align_t{alignof(Any)}));
      }
      void deallocate(gsl::not_null<pointer> ptr, count_t count)
      {
         mp_resource->deallocate(gsl::make_not_null(static_cast<memory_resource::pointer>(ptr)),
                                 count_t{sizeof(Any)} * count, align_t{alignof(Any)});
      }

      auto resource() noexcept -> memory_resource* { return mp_resource; }

   private:
      memory_resource* mp_resource{nullptr};
   };
} // namespace caramel
