#pragma once

#include <iterator>

namespace crl
{
   /*
    * @author wmbat wmbat@protonmail.com
    * @date Wednesday, 16th of december 2020
    * @brief A random access iterator
    */
   template <typename Any, bool Const = false>
   class random_access_iterator
   {
   public:
      using iterator_category = std::contiguous_iterator_tag;
      using self_type = random_access_iterator;
      using value_type = Any;
      using difference_type = std::ptrdiff_t;

   public:
      /**
       * @brief Default constructor
       */
      constexpr random_access_iterator() = default;
      /**
       * @brief Construct an iterator from a pointer to a value.
       *
       * @param[in] p_value The value the iterator will point to.
       */
      constexpr explicit random_access_iterator(value_type* p_value) : mp_value{p_value} {}

      constexpr auto operator==(self_type const& rhs) const noexcept -> bool = default;
      constexpr auto operator<=>(self_type const& rhs) const noexcept
         -> std::strong_ordering = default;

      /**
       * @brief Dereference the iterator to access the value it points to.
       *
       * @return A reference to the undelying value
       */
      constexpr auto operator*() const noexcept -> value_type& { return *mp_value; }
      /**
       * @brief Directly access the value the iterator points to.
       *
       * @return A pointer to the underlying value.
       */
      constexpr auto operator->() const noexcept -> value_type* { return mp_value; }

      /**
       * @brief Increment the iterator by one.
       *
       * @return The iterator itself.
       */
      constexpr auto operator++() noexcept -> self_type&
      {
         ++mp_value;

         return *this;
      }
      /**
       * @brief Decrement the iterator by one.
       *
       * @return The iterator itself.
       */
      constexpr auto operator--() noexcept -> self_type&
      {
         --mp_value;

         return *this;
      }

      /**
       * @brief Encrement a copy of the iterator and return it.
       *
       * @return A copy of the iterator that points to the next element.
       */
      constexpr auto operator++(int) const noexcept -> self_type
      {
         self_type it = *this;
         ++*this;

         return it;
      }
      /**
       * @brief Decrement a copy of the iterator and return it.
       *
       * @return A copy of the iterator that points to the previous element.
       */
      constexpr auto operator--(int) const noexcept -> self_type
      {
         self_type it = *this;
         --*this;

         return it;
      }

      /**
       * @brief Increment the iterator by a specified value.
       *
       * @param[in] diff
       *
       * @return The iterator itself.
       */
      constexpr auto operator+=(difference_type diff) noexcept -> self_type&
      {
         this->mp_value += diff;

         return *this;
      }
      /**
       * @brief Decrement the iterator by a specified value.
       *
       * @param[in] diff
       *
       * @return The iterator itself.
       */
      constexpr auto operator-=(difference_type diff) noexcept -> self_type&
      {
         this->mp_value -= diff;

         return *this;
      }

      constexpr auto operator+(difference_type rhs) const noexcept -> self_type
      {
         self_type it = *this;
         it += rhs;

         return it;
      }
      constexpr auto operator-(difference_type rhs) const noexcept -> self_type
      {
         self_type it = *this;
         it -= rhs;

         return it;
      }

      constexpr auto operator+(self_type const& it) const noexcept -> difference_type
      {
         return mp_value + it.mp_value;
      }
      constexpr auto operator-(self_type const& it) const noexcept -> difference_type
      {
         return mp_value - it.mp_value;
      }

      friend constexpr auto operator+(difference_type lhs, const self_type& rhs) noexcept
         -> self_type
      {
         return {rhs + lhs};
      }
      friend constexpr auto operator-(difference_type lhs, const self_type& rhs) noexcept
         -> self_type
      {
         return {rhs - lhs};
      }

      constexpr auto operator[](difference_type diff) const noexcept -> value_type&
      {
         assert(mp_value != nullptr && "Cannot use derefence a nullptr");

         return *(*this + diff);
      }

      constexpr void swap(self_type& rhs) noexcept { std::swap(mp_value, rhs.mp_value); }

   private:
      value_type* mp_value{nullptr};
   };

   template <typename Any>
   class random_access_iterator<Any, true>
   {
   public:
      using iterator_category = std::contiguous_iterator_tag;
      using self_type = random_access_iterator;
      using value_type = Any;
      using difference_type = std::ptrdiff_t;

   public:
      /**
       * @brief Default constructor
       */
      constexpr random_access_iterator() = default;
      /**
       * @brief Construct an iterator from a pointer to a value.
       *
       * @param[in] p_value The value the iterator will point to.
       */
      constexpr explicit random_access_iterator(value_type* p_value) : mp_value{p_value} {}

      constexpr auto operator==(self_type const& rhs) const noexcept -> bool = default;
      constexpr auto operator<=>(self_type const& rhs) const noexcept
         -> std::strong_ordering = default;

      /**
       * @brief Dereference the iterator to access the value it points to.
       *
       * @return A reference to the undelying value
       */
      constexpr auto operator*() const noexcept -> const value_type& { return *mp_value; }
      /**
       * @brief Directly access the value the iterator points to.
       *
       * @return A pointer to the underlying value.
       */
      constexpr auto operator->() const noexcept -> const value_type* { return mp_value; }

      /**
       * @brief Increment the iterator by one.
       *
       * @return The iterator itself.
       */
      constexpr auto operator++() noexcept -> self_type&
      {
         ++mp_value;

         return *this;
      }
      /**
       * @brief Decrement the iterator by one.
       *
       * @return The iterator itself.
       */
      constexpr auto operator--() noexcept -> self_type&
      {
         --mp_value;

         return *this;
      }

      /**
       * @brief Encrement a copy of the iterator and return it.
       *
       * @return A copy of the iterator that points to the next element.
       */
      constexpr auto operator++(int) const noexcept -> self_type
      {
         self_type it = *this;
         ++*this;

         return it;
      }
      /**
       * @brief Decrement a copy of the iterator and return it.
       *
       * @return A copy of the iterator that points to the previous element.
       */
      constexpr auto operator--(int) const noexcept -> self_type
      {
         self_type it = *this;
         --*this;

         return it;
      }

      /**
       * @brief Increment the iterator by a specified value.
       *
       * @param[in] diff
       *
       * @return The iterator itself.
       */
      constexpr auto operator+=(difference_type diff) noexcept -> self_type&
      {
         this->mp_value += diff;

         return *this;
      }
      /**
       * @brief Decrement the iterator by a specified value.
       *
       * @param[in] diff
       *
       * @return The iterator itself.
       */
      constexpr auto operator-=(difference_type diff) noexcept -> self_type&
      {
         this->mp_value -= diff;

         return *this;
      }

      constexpr auto operator+(difference_type rhs) const noexcept -> self_type
      {
         self_type it = *this;
         it += rhs;

         return it;
      }
      constexpr auto operator-(difference_type rhs) const noexcept -> self_type
      {
         self_type it = *this;
         it -= rhs;

         return it;
      }

      constexpr auto operator+(self_type const& it) const noexcept -> difference_type
      {
         return mp_value + it.mp_value;
      }
      constexpr auto operator-(self_type const& it) const noexcept -> difference_type
      {
         return mp_value - it.mp_value;
      }

      friend constexpr auto operator+(difference_type lhs, const self_type& rhs) noexcept
         -> self_type
      {
         return {rhs + lhs};
      }
      friend constexpr auto operator-(difference_type lhs, const self_type& rhs) noexcept
         -> self_type
      {
         return {rhs - lhs};
      }

      constexpr auto operator[](difference_type diff) const noexcept -> value_type&
      {
         assert(mp_value != nullptr && "Cannot use derefence a nullptr");

         return *(*this + diff);
      }

      constexpr void swap(self_type& rhs) noexcept { std::swap(mp_value, rhs.mp_value); }

   private:
      value_type* mp_value{nullptr};
   };
} // namespace crl
