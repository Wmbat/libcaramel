/**
 * @file containers/linked_array.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Sunday, 18th of may 2021
 * @brief Contains the sparse_dynamic_array API.
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCARAMEL_CONTAINERS_LINKED_ARRAY_HPP
#define LIBCARAMEL_CONTAINERS_LINKED_ARRAY_HPP

#include <libcaramel/iterators/iterator_facade.hpp>
#include <libcaramel/iterators/random_iterator.hpp>
#include <libcaramel/memory/memory_allocator.hpp>

#include <iterator>
#include <span>

namespace caramel
{
   /**
    * @brief the default size of each sub array within the basic_linked_array data structure.
    */
   constexpr i64_t linked_array_default_min_size = 8;

   /**
    * @brief
    *
    * @tparam Any The type of elements.
    * @tparam MinArrSize The size of each sub-array.
    * @tparam Allocator The allocator that is used to acquire/release and construct/destroy the
    * elements in that memory.
    */
   template <typename Element, i64_t MinArrSize, typename Allocator = memory_allocator<Element>>
   class basic_linked_array
   {
      struct group_node;

      using node_ptr = group_node*;

   public:
      using value_type = Element;
      using size_type = i64_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = Allocator;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename allocator_type::pointer;
      using const_pointer = typename allocator_type::const_pointer;

      class iterator : public iterator_facade<iterator>
      {
      public:
         iterator() = default;
         iterator(node_ptr group, difference_type offset);

         auto dereference() const noexcept -> value_type&
         {
            return *(mp_group->p_begin + m_offset);
         }

         void advance(difference_type offset) noexcept {}

      private:
         node_ptr mp_group = nullptr;
         difference_type m_offset = 0;
      };

      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      constexpr basic_linked_array() noexcept = default;
      constexpr basic_linked_array(const allocator_type& allocator) : m_allocator(allocator) {}

      constexpr auto begin() noexcept -> iterator {}
      constexpr auto begin() const noexcept -> const_iterator;
      constexpr auto cbegin() const noexcept -> const_iterator;

      constexpr auto end() noexcept -> iterator;
      constexpr auto end() const noexcept -> const_iterator;
      constexpr auto cend() const noexcept -> const_iterator;

   private:
      struct group_node
      {
         Element* p_begin = nullptr;
         Element* p_end = nullptr;

         size_type capacity = 0;

         node_ptr p_previous = nullptr;
         node_ptr p_next = nullptr;
      };

      node_ptr mp_first_node = nullptr;

      size_type m_size = 0;
      size_type m_capacity = 0;

      allocator_type m_allocator;
   };

   /**
    * @brief
    *
    * @tparam Any The type of elements.
    */
   template <typename Element>
   class linked_array
   {
      using underlying_type = basic_linked_array<Element, linked_array_default_min_size>;

   public:
   private:
   };
} // namespace caramel

#endif // LIBCARAMEL_CONTAINERS_LINKED_ARRAY_HPP
