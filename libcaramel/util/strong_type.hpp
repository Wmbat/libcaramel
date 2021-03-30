#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace caramel
{
   // clang-format off
   template <typename Any, typename parameter, template <typename> class... utils_>
   class strong_type final : public utils_<strong_type<Any, parameter, utils_...>>...
   {
   public:
      using value_type = Any;

   public:
      constexpr strong_type() 
         noexcept(std::is_nothrow_default_constructible_v<value_type>) 
         requires std::default_initializable<value_type> = default;

      constexpr explicit strong_type(const value_type& value) 
         noexcept(std::is_nothrow_copy_constructible_v<value_type>) 
         requires std::copy_constructible<value_type> 
         : m_value{value}
      {}

      constexpr explicit strong_type(value_type&& value) 
         noexcept(std::is_nothrow_move_assignable_v<value_type>)
         requires std::move_constructible<value_type>
         : m_value{std::move(value)}
      {}

      constexpr auto value() noexcept -> value_type&
      {
         return m_value;
      }
      constexpr auto value() const noexcept -> const value_type&
      {
         return m_value;
      }

      constexpr operator strong_type<value_type&, parameter>()
      {
         return strong_type<value_type&, parameter>{m_value};
      }

      struct argument
      {
         constexpr argument() = default;
         constexpr argument(argument const&) = delete;
         constexpr argument(argument&&) = delete;
         constexpr ~argument() = default;

         constexpr auto operator=(argument const&) -> argument& = delete;
         constexpr auto operator=(argument&&) -> argument& = delete;

         constexpr auto operator=(value_type&& value) const -> strong_type // NOLINT
         {
            return strong_type{std::forward<value_type>(value)};
         }

         constexpr auto operator=(auto&& value) const -> strong_type // NOLINT
         {
            return strong_type{std::forward<decltype(value)>(value)};
         }
    };

   private:
      value_type m_value;
   };
   // clang-format on

   namespace detail
   {
      template <typename Any, template <typename> class crtp_type_>
      struct crtp
      {
         constexpr auto underlying() -> Any& { return static_cast<Any&>(*this); }
         constexpr auto underlying() const -> Any const& { return static_cast<Any const&>(*this); }
      };
   } // namespace detail

   template <typename Any>
   struct pre_incrementable : detail::crtp<Any, pre_incrementable>
   {
      constexpr auto operator++() -> Any&
      {
         ++this->underlying().value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct post_incrementable : detail::crtp<Any, post_incrementable>
   {
      constexpr auto operator++(int) -> Any { return this->underlying().value()++; }
   };

   template <typename Any>
   struct incrementable : pre_incrementable<Any>, post_incrementable<Any>
   {
      using pre_incrementable<Any>::operator++;
      using post_incrementable<Any>::operator++;
   };

   template <typename Any>
   struct pre_decrementable : detail::crtp<Any, pre_decrementable>
   {
      constexpr auto operator--() -> Any&
      {
         --this->underlying().value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct post_decrementable : detail::crtp<Any, post_decrementable>
   {
      constexpr auto operator--(int) -> Any { return this->underlying().value()--; }
   };

   template <typename Any>
   struct decrementable : pre_decrementable<Any>, post_decrementable<Any>
   {
      using pre_decrementable<Any>::operator--;
      using post_decrementable<Any>::operator--;
   };

   template <typename Any>
   struct binary_addable : detail::crtp<Any, binary_addable>
   {
      constexpr auto operator+(const Any& rhs) const
      {
         return Any{this->underlying().value() + rhs.value()};
      }

      constexpr auto operator+=(const Any& rhs)
      {
         this->underlying().vale() += rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct unary_addable : detail::crtp<Any, unary_addable>
   {
      constexpr auto operator+() const { return Any{+this->undelying().value()}; }
   };

   template <typename Any>
   struct addable : binary_addable<Any>, unary_addable<Any>
   {
      using binary_addable<Any>::operator+;
      using unary_addable<Any>::operator+;
   };

   template <typename Any>
   struct binary_subtractable : detail::crtp<Any, binary_subtractable>
   {
      constexpr auto operator-(const Any& rhs) const
      {
         return Any{this->underlying().value() - rhs.value()};
      }

      constexpr auto operator-=(const Any& rhs)
      {
         this->underlying().value() -= rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct unary_subtractable : detail::crtp<Any, unary_subtractable>
   {
      constexpr auto operator-() const { return Any{-this->undelying().value()}; }
   };

   template <typename Any>
   struct subtractable : binary_subtractable<Any>, unary_subtractable<Any>
   {
      using binary_subtractable<Any>::operator-;
      using unary_subtractable<Any>::operator-;
   };

   template <typename Any>
   struct multiplicable : detail::crtp<Any, multiplicable>
   {
      constexpr auto operator*(const Any& rhs) const -> Any
      {
         return Any{this->underlying().value() * rhs.value()};
      }
      constexpr auto operator*=(const Any& rhs) -> Any&
      {
         this->underlying().value() *= rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct divisible : detail::crtp<Any, divisible>
   {
      constexpr auto operator/(const Any& other) const -> Any
      {
         return Any{this->underlying().value() / other.value()};
      }
      constexpr auto operator/=(const Any& rhs) -> Any&
      {
         this->underlying().value() /= rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct modulable : detail::crtp<Any, modulable>
   {
      constexpr auto operator%(const Any& rhs) const -> Any
      {
         return Any{this->underlying().value() % rhs.value()};
      }
      constexpr auto operator%=(const Any& other) -> Any&
      {
         this->underlying().value() %= other.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct equatable : detail::crtp<Any, equatable>
   {
      constexpr friend auto operator==(const Any& lhs, const Any& rhs) -> bool
      {
         return lhs.value() == rhs.value();
      }
   };

   template <typename Any>
   struct comparable : detail::crtp<Any, comparable>
   {
      constexpr friend auto operator<=>(const Any& lhs, const Any& rhs)
      {
         return std::compare_three_way{}(lhs, rhs);
      }
   };

   template <typename Any>
   struct arithmetic :
      incrementable<Any>,
      decrementable<Any>,
      subtractable<Any>,
      addable<Any>,
      multiplicable<Any>,
      divisible<Any>,
      modulable<Any>,
      comparable<Any>
   {
   };
} // namespace caramel
