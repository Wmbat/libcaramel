#pragma once

#include <libcaramel/iterators/iterator_facade.hpp>

#include <iterator>

namespace caramel
{
   template <typename Any>
   class contiguous_iterator : public iterator_facade<contiguous_iterator<Any>>
   {
   public:
      static constexpr bool is_contiguous_iterator = true;

   public:
      contiguous_iterator() = default;
      contiguous_iterator(Any* p_value) : mp_value(p_value) {}

      [[nodiscard]] auto dereference() const noexcept -> Any& { return *mp_value; }

      void advance(std::ptrdiff_t off) noexcept { mp_value += off; }
      [[nodiscard]] auto distance_to(contiguous_iterator other) const noexcept -> std::ptrdiff_t
      {
         return other.mp_value - mp_value;
      }
      auto operator==(contiguous_iterator other) const noexcept -> bool
      {
         return other.mp_value == mp_value;
      }

   private:
      Any* mp_value{nullptr};
   };
} // namespace caramel
