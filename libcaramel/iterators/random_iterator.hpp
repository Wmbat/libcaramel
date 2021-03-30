#pragma once

#include <iterator>

namespace caramel
{
   template <typename Any>
   class random_access_iterator
   {
   public:
      using iterator_category = std::random_access_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = Any;
      using reference = value_type&;
      using pointer = value_type*;

   public:
      random_access_iterator() = default;
      random_access_iterator(pointer value) : mp_value{value} {}

      constexpr auto operator<=>(const random_access_iterator& rhs) const noexcept
         -> std::strong_ordering = default;

      constexpr auto operator*() const noexcept -> reference { return *mp_value; }
      constexpr auto operator->() const noexcept -> pointer { return mp_value; }

      constexpr auto operator++() noexcept -> random_access_iterator&
      {
         ++mp_value;

         return *this;
      }
      constexpr auto operator--() noexcept -> random_access_iterator&
      {
         --mp_value;

         return *this;
      }

      constexpr auto operator++(int) const noexcept -> random_access_iterator
      {
         random_access_iterator it = *this;
         ++*this;

         return it;
      }

      constexpr auto operator--(int) const noexcept -> random_access_iterator
      {
         random_access_iterator it = *this;
         --*this;

         return it;
      }

      constexpr auto operator+=(difference_type diff) noexcept -> random_access_iterator&
      {
         this->mp_value += diff;

         return *this;
      }

      constexpr auto operator-=(difference_type diff) noexcept -> random_access_iterator&
      {
         this->mp_value -= diff;

         return *this;
      }

      constexpr auto operator+(difference_type rhs) const noexcept -> random_access_iterator
      {
         random_access_iterator it = *this;
         it += rhs;

         return it;
      }
      constexpr auto operator-(difference_type rhs) const noexcept -> random_access_iterator
      {
         random_access_iterator it = *this;
         it -= rhs;

         return it;
      }

      constexpr auto operator+(random_access_iterator const& it) const noexcept -> difference_type
      {
         return mp_value + it.mp_value;
      }
      constexpr auto operator-(random_access_iterator const& it) const noexcept -> difference_type
      {
         return mp_value - it.mp_value;
      }

      friend constexpr auto operator+(difference_type lhs,
                                      const random_access_iterator& rhs) noexcept
         -> random_access_iterator
      {
         return {rhs + lhs};
      }
      friend constexpr auto operator-(difference_type lhs,
                                      const random_access_iterator& rhs) noexcept
         -> random_access_iterator
      {
         return {rhs - lhs};
      }

      constexpr auto operator[](difference_type diff) const noexcept -> value_type&
      {
         assert(mp_value != nullptr && "Cannot use derefence a nullptr");

         return *(*this + diff);
      }

      constexpr void swap(random_access_iterator& rhs) noexcept
      {
         std::swap(mp_value, rhs.mp_value);
      }

   private:
      pointer mp_value{nullptr};
   };
} // namespace caramel
