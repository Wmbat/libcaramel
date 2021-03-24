#pragma once

#include <bits/iterator_concepts.h>
#include <iterator>

namespace crl
{
   template <typename Any>
   class contiguous_iterator
   {
   public:
      using iterator_category = std::contiguous_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = Any;
      using reference = value_type&;
      using pointer = value_type*;

   public:
      contiguous_iterator() = default;
      contiguous_iterator(pointer value) : mp_value{value} {}

      constexpr auto operator<=>(const contiguous_iterator& rhs) const noexcept
         -> std::strong_ordering = default;

      constexpr auto operator*() const noexcept -> reference { return *mp_value; }
      constexpr auto operator->() const noexcept -> pointer { return mp_value; }

      constexpr auto operator++() noexcept -> contiguous_iterator&
      {
         ++mp_value;

         return *this;
      }
      constexpr auto operator--() noexcept -> contiguous_iterator&
      {
         --mp_value;

         return *this;
      }

      constexpr auto operator++(int) const noexcept -> contiguous_iterator
      {
         contiguous_iterator it = *this;
         ++*this;

         return it;
      }

      constexpr auto operator--(int) const noexcept -> contiguous_iterator
      {
         contiguous_iterator it = *this;
         --*this;

         return it;
      }

      constexpr auto operator+=(difference_type diff) noexcept -> contiguous_iterator&
      {
         this->mp_value += diff;

         return *this;
      }

      constexpr auto operator-=(difference_type diff) noexcept -> contiguous_iterator&
      {
         this->mp_value -= diff;

         return *this;
      }

      constexpr auto operator+(difference_type rhs) const noexcept -> contiguous_iterator
      {
         contiguous_iterator it = *this;
         it += rhs;

         return it;
      }
      constexpr auto operator-(difference_type rhs) const noexcept -> contiguous_iterator
      {
         contiguous_iterator it = *this;
         it -= rhs;

         return it;
      }

      constexpr auto operator+(contiguous_iterator const& it) const noexcept -> difference_type
      {
         return mp_value + it.mp_value;
      }
      constexpr auto operator-(contiguous_iterator const& it) const noexcept -> difference_type
      {
         return mp_value - it.mp_value;
      }

      friend constexpr auto operator+(difference_type lhs, const contiguous_iterator& rhs) noexcept
         -> contiguous_iterator
      {
         return {rhs + lhs};
      }
      friend constexpr auto operator-(difference_type lhs, const contiguous_iterator& rhs) noexcept
         -> contiguous_iterator
      {
         return {rhs - lhs};
      }

      constexpr auto operator[](difference_type diff) const noexcept -> value_type&
      {
         assert(mp_value != nullptr && "Cannot use derefence a nullptr");

         return *(*this + diff);
      }

      constexpr void swap(contiguous_iterator& rhs) noexcept { std::swap(mp_value, rhs.mp_value); }

   private:
      pointer mp_value{nullptr};
   };
} // namespace crl
