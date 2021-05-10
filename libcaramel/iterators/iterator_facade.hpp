#pragma once

#include <libcaramel/util/crtp.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace caramel
{
   namespace detail
   {
      // clang-format off
      
      template <typename T>
      struct wrap_refs
      {
         using type = T;
      };

      template <typename T>
         requires std::is_reference_v<T> 
      struct wrap_refs<T>
      {
         using type = std::reference_wrapper<std::remove_reference_t<T>>;
      };
      // clang-format on

      template <typename T>
      using wrap_refs_t = typename wrap_refs<T>::type;

      template <typename T>
      constexpr auto unref(T&& t) noexcept -> T&&
      {
         return static_cast<T&&>(t);
      }

      template <typename T>
      constexpr auto unref(std::reference_wrapper<T> t) noexcept -> T&
      {
         return t;
      }

      template <typename Any>
      class arrow_proxy
      {
         wrap_refs_t<Any> m_value;

      public:
         explicit constexpr arrow_proxy(Any&& t) noexcept : m_value(std::forward<decltype(t)&&>(t))
         {}

         constexpr auto operator*() noexcept -> auto& { return unref(m_value); }
         constexpr auto operator*() const noexcept -> auto& { return unref(m_value); }

         constexpr auto operator->() noexcept { return std::addressof(**this); }
         constexpr auto operator->() const noexcept { return std::addressof(**this); }
      };

      template <typename T>
      arrow_proxy(T&&) -> arrow_proxy<T>;

      struct iterator_facade_base
      {
      };

      // clang-format off

      template <typename Sentinel, typename Iter>
      concept sized_sentinel_of = requires(const Iter& it, const Sentinel& sentinel) 
      {
         it.distance_to(sentinel);
      };

      template <typename>
      struct infer_difference_type 
      {
         using type = std::ptrdiff_t;
      };

      template <typename Any>
         requires sized_sentinel_of<Any, Any>
      struct infer_difference_type<Any>
      {
         static const Any& _it;
         using type = decltype(_it.distance_to(_it));
      };

      template <typename Any>
      using infer_difference_type_t = typename infer_difference_type<Any>::type;


      template <typename Any>
      struct infer_value_type
      {
         static const Any& _it; 
         using type = std::remove_const_t<std::remove_reference_t<decltype(*_it)>>;
      };

      template <typename Any>
         requires requires { typename Any::value_type; }
      struct infer_value_type<Any>
      {
         using type = typename Any::value_type;
      };

      template <typename Any>
      using infer_value_type_t = typename infer_value_type<Any>::type;

      template<typename Any>
      concept can_increment = requires(Any& v) 
      {
         v.increment();
      };

      template<typename Any>
      concept can_decrement = requires(Any& v) 
      {
         v.decrement();
      };

      template <typename Any>
      concept can_advance = requires(Any& v, const infer_difference_type_t<Any> d)
      {
         v.advance(d);
      };

      template <typename Any>
      concept random_access_iter = sized_sentinel_of<Any, Any> and can_advance<Any>;

      template <typename Any>
      concept bidirectional_iter = random_access_iter<Any> or can_decrement<Any>;

      template <typename Any>
      concept single_pass_iter = bool(Any::single_pass_iterator);

      template <typename Any, typename Iter>
      concept iter_diff = std::is_convertible_v<Any, infer_difference_type_t<Iter>>;
      // clang-format on
   } // namespace detail

   template <typename Child>
   class iterator_facade : public crtp<Child, iterator_facade>, public detail::iterator_facade_base
   {
   public:
      constexpr auto operator==(const Child& rhs) const -> bool
      {
         return this->underlying().equal_to(rhs);
      }

      constexpr decltype(auto) operator*() const { return this->underlying().dereference(); }
      constexpr auto operator->() const
      {
         decltype(auto) ref = **this;
         if constexpr (std::is_reference_v<decltype(ref)>)
         {
            return std::addressof(ref);
         }
         else
         {
            return detail::arrow_proxy{std::move(ref)};
         }
      }

      constexpr auto operator++() -> Child&
      {
         if constexpr (detail::can_increment<Child>)
         {
            this->underlying().increment();
         }
         else if constexpr (detail::random_access_iter<Child>)
         {
            this->underlying() += 1;
         }
         else
         {
            static_assert(detail::random_access_iter<Child>,
                          "Iterator subclass must provide an `increment` or `advance(n)` method");
         }

         return this->underlying();
      }

      constexpr auto operator++(int) // NOLINT
      {
         if constexpr (detail::single_pass_iter<Child>)
         {
            ++*this;
         }
         else
         {
            auto it = this->underlying();
            ++*this;

            return it;
         }
      }

      constexpr auto operator--() -> Child& requires detail::bidirectional_iter<Child>
      {
         if constexpr (detail::can_decrement<Child>)
         {
            this->underlying().decrement();
         }
         else
         {
            this->underlying() -= 1;
         }

         return this->underlying();
      }

      constexpr auto operator--(int) -> Child requires detail::bidirectional_iter<Child>
      {
         auto it = this->underlying();
         --*this;

         return it;
      }

      template <detail::iter_diff<Child> Diff>
      friend constexpr auto operator+(const Child& left, Diff off) noexcept
         requires detail::random_access_iter<Child>
      {
         auto cp = left;
         return cp += off;
      }

      template <detail::iter_diff<Child> D>
      friend constexpr auto operator+(D off, const Child& self) noexcept
         requires detail::random_access_iter<Child>
      {
         return self + off;
      }

      friend constexpr auto operator-(const Child& left,
                                      const Child& right) requires detail::random_access_iter<Child>
      {
         return right.distance_to(left);
      }

      template <detail::iter_diff<Child> D>
      friend constexpr auto operator-(const Child& self, D off) noexcept
         requires detail::random_access_iter<Child>
      {
         using diff_type = detail::infer_difference_type_t<Child>;
         using signed_diff_type = std::make_signed_t<diff_type>;
         return self + -static_cast<signed_diff_type>(off);
      }

      template <detail::sized_sentinel_of<Child> S>
      friend constexpr auto operator-(const S& s, const Child& self) noexcept
      {
         return self.distance_to(s);
      }

      template <detail::iter_diff<Child> D>
      constexpr friend auto operator+=(Child& self, D off) noexcept
         -> Child& requires detail::random_access_iter<Child>
      {
         self.advance(off);
         return self;
      }

      template <detail::iter_diff<Child> D>
      constexpr friend auto operator-=(Child& self, D off) noexcept
         -> Child& requires detail::random_access_iter<Child>
      {
         return self = self - off;
      }

      template <detail::iter_diff<Child> D>
      [[nodiscard]] constexpr decltype(auto) operator[](D pos) const noexcept
         requires detail::random_access_iter<Child>
      {
         return *(this->underlying() + pos);
      }

      template <std::convertible_to<const Child&> Self, detail::sized_sentinel_of<Self> S>
      friend constexpr auto operator<=>(const Self& self, const S& right) noexcept
      {
         auto dist = self - right;
         auto rel = dist <=> 0;
         return rel;
      }
   };
} // namespace caramel

namespace std
{
   template <std::derived_from<caramel::detail::iterator_facade_base> Derived>
   struct iterator_traits<Derived>
   {
      static const Derived& _const_it;

      using value_type = caramel::detail::infer_value_type_t<Derived>;
      using difference_type = caramel::detail::infer_difference_type_t<Derived>;
      using reference = decltype(*_const_it);
      using pointer = decltype(_const_it.operator->());

      // Pick the iterator category based on the interfaces that it provides
      using iterator_category = std::conditional_t<
         // Random access?
         caramel::detail::random_access_iter<Derived>, std::random_access_iterator_tag,
         // Nope
         std::conditional_t<
            // Bidirectional?
            caramel::detail::bidirectional_iter<Derived>, std::bidirectional_iterator_tag,
            // Noh
            std::conditional_t<
               // Is it single-pass?
               caramel::detail::single_pass_iter<Derived>,
               // Than means it is an input iterator
               std::input_iterator_tag,
               // Otherwise it is a forward iterator
               std::forward_iterator_tag>>>;

      using iterator_concept = iterator_category;
   };
} // namespace std
