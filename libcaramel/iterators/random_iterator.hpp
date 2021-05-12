#pragma once

#include <libcaramel/iterators/iterator_facade.hpp>

#include <iterator>

namespace caramel
{
   template <typename Any>
   class random_access_iterator : public iterator_facade<random_access_iterator<Any>>
   {
   public:
      random_access_iterator() = default;
      random_access_iterator(Any* p_value) : mp_value(p_value) {}

      [[nodiscard]] auto dereference() const noexcept -> Any& { return *mp_value; }

      void advance(std::ptrdiff_t off) noexcept { mp_value += off; }
      [[nodiscard]] auto distance_to(random_access_iterator other) const noexcept -> std::ptrdiff_t
      {
         return other.mp_value - mp_value;
      }
      auto operator==(random_access_iterator other) const noexcept -> bool
      {
         return other.mp_value == mp_value;
      }

   private:
      Any* mp_value{nullptr};
   };
} // namespace caramel
