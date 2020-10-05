#pragma once

#include <cstdint>
#include <memory>

namespace crl
{
   namespace pmr
   {
      template <typename Any, std::size_t Size,
                typename Allocator = std::pmr::polymorphic_allocator<Any>>
      class small_dynamic_array
      {
         using allocator_traits = std::allocator_traits<Allocator>;

         static inline constexpr bool is_nothrow_swappable =
            std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
            std::allocator_traits<Allocator>::is_always_equal::value;

      public:
         using value_type = Any;
         using size_type = std::size_t;
         using difference_type = std::ptrdiff_t;
         using allocator_type = Allocator;
         using reference = value_type&;
         using const_reference = const value_type&;
         using pointer = typename std::allocator_traits<allocator_type>::pointer;
         using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
         using iterator = pointer;
         using const_iterator = const_pointer;
         using reverse_iterator = std::reverse_iterator<iterator>;
         using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      public:
         constexpr small_dynamic_array() noexcept(std::is_nothrow_constructible_v<allocator_type>);
         constexpr explicit small_dynamic_array(const allocator_type& alloc) noexcept;

         constexpr auto allocator() const noexcept -> allocator_type;

         constexpr auto begin() noexcept -> iterator;
         constexpr auto begin() const noexcept -> const_iterator;
         constexpr auto cbegin() const noexcept -> const_iterator;

         constexpr auto end() noexcept -> iterator;
         constexpr auto end() const noexcept -> const_iterator;
         constexpr auto cend() const noexcept -> const_iterator;

         constexpr auto rbegin() noexcept -> reverse_iterator;
         constexpr auto rbegin() const noexcept -> const_reverse_iterator;
         constexpr auto rcbegin() const noexcept -> const_reverse_iterator;

         constexpr auto rend() noexcept -> reverse_iterator;
         constexpr auto rend() const noexcept -> const_reverse_iterator;
         constexpr auto rcend() const noexcept -> const_reverse_iterator;

         constexpr auto empty() const noexcept -> bool;
         constexpr auto size() const noexcept -> size_type;
         constexpr auto capacity() const noexcept -> size_type;

         constexpr auto insert(const_iterator position, const value_type& value);
         constexpr auto insert(const_iterator position, value_type&& value);

         constexpr auto erase(const_iterator position) -> iterator;
         constexpr auto erase(const_iterator first, const_iterator last) -> iterator;

      private:
      };
   } // namespace pmr

   template <typename Any, std::size_t Size>
   class small_dynamic_array
   {
   public:
   private:
      pmr::small_dynamic_array<Any, Size, std::pmr::polymorphic_allocator<Any>> underlying;
   };

   template <typename Any>
   class dynamic_array
   {
   public:
   private:
      pmr::small_dynamic_array<Any, 0> underlying;
   };
} // namespace crl
