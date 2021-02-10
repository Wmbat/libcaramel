#pragma once

#include <libcaramel/assert.hpp>
#include <libcaramel/iterators/random_access.hpp>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <memory_resource>
#include <type_traits>

namespace crl
{
   namespace detail
   {
      template <typename First, typename Second>
      auto synth_three_way(const First& lhs, const Second& rhs)
      {
         if constexpr (std::three_way_comparable_with<First, Second>)
         {
            return lhs <=> rhs;
         }
         else
         {
            if (lhs == rhs)
            {
               return std::strong_ordering::equal;
            }

            if (lhs < rhs)
            {
               return std::strong_ordering::less;
            }

            return std::strong_ordering::greater;
         }
      }
   } // namespace detail

   /**
    * @author wmbat wmbat@protonmail.com
    * @date Sunday, 13th of december 2020
    * @brief A resizable array with a small statically allocated storage buffer
    * @copyright MIT License
    *
    * @tparam Any The type of the elements
    * @tparam Size The size of the staticly allocated small buffer.
    * @tparam Allocator The allocator that is used to acquire/release and construct/destroy the
    * elements in that memory.
    */
   template <typename Any, std::size_t Size,
             typename Allocator = std::pmr::polymorphic_allocator<Any>>
   class basic_dynamic_array
   {
      using allocator_traits = std::allocator_traits<Allocator>;

   public:
      using value_type = Any;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = Allocator;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<pointer>;
      using const_iterator = random_access_iterator<const_pointer>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      /**
       * @brief Default constructor.
       */
      constexpr basic_dynamic_array() noexcept(noexcept(allocator_type{})) = default;
      /**
       * @brief Default construct the container with a given allocator
       *
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      constexpr basic_dynamic_array(const allocator_type& allocator) : m_allocator{allocator} {}
      /**
       * @brief Construct the container with count copies of elements with value value
       *
       * @param[in] count The size of the container.
       * @param[in] The value to initialize elements from.
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      constexpr basic_dynamic_array(size_type count, const_reference value,
                                    const allocator_type& allocator = allocator_type{}) :
         m_allocator{allocator}
      {
         assign(count, value);
      }
      /**
       * @brief Construct the container with the contents of the initializer list init.
       *
       * @param[in] init Initializer list to initialize the elements of the container with.
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      constexpr basic_dynamic_array(std::initializer_list<Any> init,
                                    const allocator_type& allocator = allocator_type{}) :
         m_allocator{allocator}
      {
         assign(init);
      }
      /**
       * @brief Construct the container with the contents of the range [first, last)
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      template <std::input_iterator InputIt>
      constexpr basic_dynamic_array(InputIt first, InputIt last,
                                    const allocator_type& allocator = allocator_type{}) :
         m_allocator{allocator}
      {
         assign(first, last);
      }
      /**
       * @brief Construct the container using the contents of other.
       *
       * @param[in] other Another container to be used as source to initialize the elements of the
       * container with.
       */
      constexpr basic_dynamic_array(const basic_dynamic_array& other)
      {
         if (this != &other)
         {
            *this = other;
         }
      }
      /**
       * @brief Construct the container using the contents of other. using allocator as the
       * allocator.
       *
       * @param[in] other Another container to be used as source to initialize the elements of the
       * container with.
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      constexpr basic_dynamic_array(const basic_dynamic_array& other,
                                    const allocator_type& allocator)
      {
         if (this != &other)
         {
            clear();

            if (!is_static())
            {
               allocator_traits::deallocate(m_allocator, mp_begin, capacity());
            }

            reset_to_static();

            m_allocator = allocator;

            const size_type new_count = std::distance(std::begin(other), std::end(other));
            if (new_count > capacity())
            {
               grow(new_count);
            }

            m_size = new_count;

            std::uninitialized_copy(std::begin(other), std::end(other), begin());
         }
      }
      /**
       * @brief Construct the container with the contents of the other using move semantic. After
       * move, other is guarenteed to be empty().
       *
       * @param[in] other another container to be used as source to initialize the elements of the
       * container with.
       */
      constexpr basic_dynamic_array(basic_dynamic_array&& other) noexcept
      {
         if (!other.empty())
         {
            *this = std::move(other);
         }
      }
      /**
       * @brief Construct the container with the contents of the other using move semantic. Using
       * alloc as the allocator for the new container.
       *
       * @param[in] other another container to be used as source to initialize the elements of the
       * container with.
       * @param[in] allocator The allocator to use for all memory allocations of this container.
       */
      constexpr basic_dynamic_array(basic_dynamic_array&& other, const allocator_type& alloc)
      {
         clear();

         if (!is_static())
         {
            allocator_traits::deallocate(m_allocator, mp_begin, capacity());
         }

         reset_to_static();

         m_allocator = alloc;

         using mi = std::move_iterator<iterator>;
         assign(mi{other.begin()}, mi{other.end()});

         other.reset_to_static();
      }
      /**
       * @brief Destructor
       */
      constexpr ~basic_dynamic_array() noexcept
      {
         clear();

         if (!is_static() && mp_begin)
         {
            allocator_traits::deallocate(m_allocator, mp_begin, capacity());
         }

         reset_to_static();
      }

      /**
       * @brief Replaces the contents with an copy of the contents of rhs.
       *
       * @param[in] rhs other container to use as a data source.
       */
      constexpr auto operator=(const basic_dynamic_array& rhs) -> basic_dynamic_array&
      {
         if (this != &rhs)
         {
            copy_assign_alloc(rhs);
            assign(rhs.begin(), rhs.end());
         }

         return *this;
      }

      /**
       * @brief Replaces the contents with those of other using move semantics.
       *
       * @param[in] rhs other container to use as a data source.
       */
      constexpr auto operator=(basic_dynamic_array&& rhs) noexcept(
         allocator_traits::propagate_on_container_move_assignment::value ||
         allocator_traits::is_always_equal::value) -> basic_dynamic_array&
      {
         move_assign(rhs,
                     std::integral_constant<
                        bool, allocator_traits::propagate_on_container_move_assignment::value>());

         return *this;
      }

      /**
       * @brief Replaces the contents with those identified by initializer list init_list
       *
       * @param init_list Initializer list to use as data source.
       */
      constexpr auto operator=(std::initializer_list<Any> init_list) -> basic_dynamic_array&
      {
         assign(init_list);

         return *this;
      }

      /**
       * @brief Returns the allocator associated with the container.
       *
       * @return The associated allocator.
       */
      constexpr auto allocator() const noexcept -> allocator_type { return m_allocator; }

      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) -> reference
      {
         EXPECT(index < m_size);

         return mp_begin[index];
      }
      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A const reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) const -> const_reference
      {
         EXPECT(index < m_size);

         return mp_begin[index];
      }

      /**
       * @brief Access the data stored by the container.
       *
       * @return A pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() noexcept -> pointer { return pointer{&(*begin())}; }
      /**
       * @brief Access the data stored by the container.
       *
       * @return A const_pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() const noexcept -> const_pointer { return const_pointer{&(*cbegin())}; }

      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return iterator{mp_begin}; }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return const_iterator{mp_begin}; }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return iterator to the first element. If the basic_dynamic_array is empty, the
       * const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return const_iterator{mp_begin}; }

      /**
       * @brief Get an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() noexcept -> iterator { return iterator{mp_begin + m_size}; }
      /**
       * @brief Return an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() const noexcept -> const_iterator
      {
         return const_iterator{mp_begin + m_size};
      }
      /**
       * @brief Returns an it iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto cend() const noexcept -> const_iterator
      {
         return const_iterator{mp_begin + m_size};
      }

      /**
       * @brief Returns a reverse iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }
      /**
       * @brief Returns a reverse_iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return reverse_iterator to the first element.
       */
      constexpr auto rbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }
      /**
       * @brief Returns a reverse iterator to the first element of the reversed
       * basic_dynamic_array. It corresponds to the last element of the non-reversed
       * basic_dynamic_array. If the basic_dynamic_array is empty, the returned iterator is equal to
       * rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }

      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rcend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }

      /**
       * @brief Check if the basic_dynamic_array is empty.
       *
       * @return True if the container is empty, false otherwise.
       */
      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return begin() == end(); };
      /**
       * @brief Check the number of elements stored in the basic_dynamic_array.
       *
       * @return The number of elements in the basic_dynamic_array.
       */
      [[nodiscard]] constexpr auto size() const noexcept -> size_type { return m_size; };
      /**
       * @brief Check the number of elements that the basic_dynamic_array has currently allocated
       * space for.
       *
       * @return Capacity of the currently allocated storage.
       */
      [[nodiscard]] constexpr auto capacity() const noexcept -> size_type { return m_capacity; };
      /**
       * @brief Increase the capacity of the vector to a value that's greater or equal to new_cap.
       * If new_ap is greater than the current capacity(), new storage is allocated, otherwise the
       * method does nothing. If reallocated occurs, all current iterators are invalidated.
       *
       * @param new_cap New capacity of the vector.
       *
       * @throws If the undelying allocator failed to allocate memory.
       */
      constexpr void reserve(size_type new_cap)
      {
         if (new_cap > capacity())
         {
            grow(new_cap);
         }
      }

      /**
       * @brief Erases all elements from the container, After this call, size() returs zero.
       */
      constexpr void clear() noexcept
      {
         destroy(begin(), end());
         m_size = 0;
      }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, const_reference value) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         if (pos == cend())
         {
            append(value);

            return end() - 1;
         }

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         construct(offset(size()), std::move(*(end() - 1)));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         const_pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = *p_element;

         return new_pos;
      }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, value_type&& value) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         if (pos == cend())
         {
            append(std::move(value));

            return end() - 1;
         }

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         construct(offset(size()), std::move(*(end() - 1)));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = std::move(*p_element);

         return new_pos;
      }
      /**
       * @brief Insert a new element into the container directly before pos. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * @param[in] args Arguments to forward to the constructor of the element.
       *
       * @return Iterator pointing to the inserted value.
       */
      template <typename... Args>
      constexpr auto insert(const_iterator pos, Args... args) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         if (pos == cend())
         {
            append(std::forward<Args>(args)...);

            return end() - 1;
         }

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         new (&(*end())) value_type(std::move(*(end() - 1)));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         *new_pos = value_type(std::forward<Args>(args)...);

         return new_pos;
      }
      /**
       * @brief Inserts count elements from a specified value.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] count The number of elements to insert.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, size_type count, const_reference value) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         size_type start_index = pos - cbegin();

         if (pos == cend())
         {
            if (size() + count >= capacity())
            {
               grow(size() + count);
            }

            std::uninitialized_fill_n(end(), count, value);

            m_size += count;

            return begin() + start_index;
         }

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;

         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            m_size += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::fill_n(updated_pos, count, value);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            m_size += count;

            std::uninitialized_move(updated_pos, old_end, end() - move_count);
            std::fill_n(updated_pos, move_count, value);
            std::uninitialized_fill_n(old_end, count - move_count, value);
         }

         return updated_pos;
      }

      /**
       * @brief Inserts elements from a range [first, last) before pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] first The first value to insert
       * @param[in] last One past the last value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      template <std::input_iterator InputIt>
      constexpr auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         size_type start_index = pos - cbegin();
         difference_type count = std::distance(first, last);

         if (pos == cend())
         {
            if (size() + count >= capacity())
            {
               grow(size() + count);
            }

            std::uninitialized_copy(first, last, end());

            m_size += count;

            return begin() + start_index;
         }

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;
         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            m_size += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::copy(first, last, updated_pos);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            m_size += count;

            std::uninitialized_move(updated_pos, old_end, end() - move_count);

            for (auto it = updated_pos; count > 0; --count)
            {
               *it = *first;

               ++it;
               ++first;
            }

            std::uninitialized_copy(first, last, old_end);
         }

         return updated_pos;
      }

      /**
       * @brief Insert elements from an initializer_list before the position pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] init_list Initializer list to insert the values from.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, std::initializer_list<value_type> init_list)
         -> iterator
      {
         return insert(pos, init_list.begin(), init_list.end());
      }

      /**
       * @brief Erases the specified element from the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator to the element to remove.
       */
      constexpr auto erase(const_iterator pos) -> iterator
      {
         EXPECT(pos >= cbegin());
         EXPECT(pos <= cend());

         if (pos == cend())
         {
            return end();
         }

         auto it = begin() + (pos - cbegin());

         std::move(it + 1, end(), it);

         pop_back();

         return it;
      }
      /**
       * @brief Erases the specified elements from the container.
       *
       * @pre first >= begin()
       * @pre last <= end()
       * @pre first >= last
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       *
       * @return Iterator following the last removed element. If last == end() prior to removal,
       * then the updated end() iterator is returned. If [first, last) is an empty range, the last
       * iterator is returned.
       */
      constexpr auto erase(const_iterator first, const_iterator last) -> iterator
      {
         EXPECT(first >= cbegin());
         EXPECT(last <= cend());
         EXPECT(first >= last);

         if (first == last)
         {
            return begin() + (first - cbegin());
         }

         size_type const distance = std::distance(first, last);

         iterator it_f = begin() + (first - cbegin());
         iterator it_l = begin() + (last - cbegin());
         iterator it = std::move(it_l, end(), it_f);

         destroy(it, end());

         m_size -= distance;

         return it_f;
      }

      /**
       * @brief Appends the given element value to the end of the container. The new element is
       * initialized as a copy of value.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(const value_type& value)
      {
         if (size() >= capacity())
         {
            grow();
         }

         construct(mp_begin + size(), value);

         ++m_size;
      }
      /**
       * @brief Appends the given element value to the end of the container. Value is moved into the
       * new element.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(value_type&& value)
      {
         if (size() >= capacity())
         {
            grow();
         }

         construct(mp_begin + size(), std::move(value));

         ++m_size;
      }
      /**
       * @brief Appends the given element value to the end of the container. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @param[in] args Arguments to forward to the constructor of the element.
       */
      template <typename... Args>
      constexpr auto append(Args&&... args) -> reference
      {
         if (size() >= capacity())
         {
            grow();
         }

         construct(mp_begin + size(), std::forward<Args>(args)...);

         ++m_size;

         return *(end() - 1);
      }

      /**
       * @brief Removes the last element in the container.
       *
       * @pre size() != 0
       */
      constexpr void pop_back()
      {
         EXPECT(size() != 0);

         destroy(offset(size()));
         --m_size;
      };

      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, default constructed elements are appended.
       *
       * @param[in] count New size of the container.
       */
      constexpr void resize(size_type count)
      {
         if (size() > count)
         {
            destroy(begin() + count, end());
            m_size = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            for (size_type i = size(); i < count; ++i)
            {
               construct(offset(i), value_type{});
            }

            m_size = count;
         }
      }
      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, additional copies of value are appended.
       *
       * @param[in] count New size of the container.
       * @param[in] value The value to initialize new elements with.
       */
      constexpr void resize(size_type count, const_reference value)
      {
         if (size() > count)
         {
            destroy(begin() + count, end());
            m_size = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            std::uninitialized_fill(end(), begin() + count, value);

            m_size = count;
         }
      }

   private:
      [[nodiscard]] constexpr auto is_static() const noexcept -> bool
      {
         return mp_begin == get_first_element();
      }

      constexpr auto get_first_element() const -> pointer
      {
         return const_cast<pointer>(reinterpret_cast<const_pointer>(&m_static_storage)); // NOLINT
      }

      constexpr void grow(size_type min_size = 0)
      {
         EXPECT(min_size > std::numeric_limits<difference_type>::max());
         EXPECT(capacity() == std::numeric_limits<difference_type>::max());

         const auto new_capacity = compute_new_capacity(std::max(m_capacity + 1, min_size));
         auto new_elements = allocator_traits::allocate(m_allocator, new_capacity);

         if constexpr (std::is_move_constructible_v<value_type>)
         {
            std::uninitialized_move(begin(), end(), iterator{new_elements});
         }
         else
         {
            std::uninitialized_copy(begin(), end(), iterator{new_elements});
         }

         destroy(begin(), end());

         if (!is_static())
         {
            allocator_traits::deallocate(m_allocator, mp_begin, capacity());
         }

         mp_begin = new_elements;
         m_capacity = new_capacity;
      }

      constexpr void reset_to_static()
      {
         mp_begin = get_first_element();
         m_size = 0;
         m_capacity = Size;
      }

      template <typename... Args>
      constexpr void construct(pointer p_loc, Args&&... args)
      {
         if (is_static())
         {
            std::construct_at(p_loc, std::forward<Args>(args)...);
         }
         else
         {
            allocator_traits::construct(m_allocator, p_loc, std::forward<Args>(args)...);
         }
      }

      constexpr void destroy(pointer p_loc)
      {
         if (is_static())
         {
            std::destroy_at(p_loc);
         }
         else
         {
            allocator_traits::destroy(m_allocator, p_loc);
         }
      }
      constexpr void destroy(iterator beg, iterator end)
      {
         if (is_static())
         {
            std::destroy(beg, end);
         }
         else
         {
            std::for_each(beg, end, [&](auto& value) {
               allocator_traits::destroy(m_allocator, std::addressof(value));
            });
         }
      }
      constexpr void copy_assign_alloc(const basic_dynamic_array& other)
      {
         if constexpr (allocator_traits::propagate_on_container_copy_assignment::value)
         {
            if (m_allocator != other.m_allocator)
            {
               clear();

               if (!is_static())
               {
                  allocator_traits::deallocate(m_allocator, mp_begin, capacity());
               }

               reset_to_static();
            }

            m_allocator = other.m_allocator;
         }
      }

      constexpr void move_assign_alloc(const basic_dynamic_array& other)
      {
         if constexpr (allocator_traits::propagate_on_container_move_assignment::value)
         {
            m_allocator = std::move(other.m_allocator);
         }
      }

      constexpr void move_assign(basic_dynamic_array& other, std::false_type /*u*/)
      {
         if (m_allocator != other.m_allocator)
         {
            using mi = std::move_iterator<iterator>;
            assign(mi{other.begin()}, mi{other.end()});

            other.reset_to_static();
         }
         else
         {
            move_assign(other, std::true_type{});
         }
      }
      constexpr void move_assign(basic_dynamic_array& other, std::true_type /*u*/)
      {
         if (!other.is_static())
         {
            if (!is_static())
            {
               clear();

               allocator_traits::deallocate(m_allocator, mp_begin, capacity());

               reset_to_static();
            }

            move_assign_alloc(other);

            m_size = other.size();
            m_capacity = other.capacity();
            mp_begin = other.mp_begin;
         }
         else
         {
            using mi = std::move_iterator<iterator>;
            assign(mi{other.begin()}, mi{other.end()});

            move_assign_alloc(other);
         }

         other.reset_to_static();
      }

      constexpr void assign(size_type count, const_reference value)
      {
         clear();

         if (count > m_capacity)
         {
            grow(count);
         }

         m_size = count;

         std::uninitialized_fill(begin(), end(), value);
      }

      template <std::input_iterator InputIt>
      constexpr void assign(InputIt first, InputIt last)
      {
         clear();

         const auto new_count = static_cast<size_type>(std::distance(first, last));
         if (new_count > capacity())
         {
            grow(new_count);
         }

         m_size = new_count;

         std::uninitialized_copy(first, last, begin());
      }

      constexpr void assign(std::initializer_list<value_type> initializer_list)
      {
         assign(initializer_list.begin(), initializer_list.end());
      }

      constexpr auto offset(size_type i) noexcept -> pointer { return mp_begin + i; }
      constexpr auto offset(size_type i) const noexcept -> const_pointer { return mp_begin + i; }

      static constexpr auto compute_new_capacity(size_type min_capacity) -> size_type
      {
         constexpr auto max_capacity = size_type{1} << (std::numeric_limits<size_type>::digits - 1);

         if (min_capacity > max_capacity)
         {
            return max_capacity;
         }

         --min_capacity;

         for (auto i = 1U; i < std::numeric_limits<size_type>::digits; i *= 2)
         {
            min_capacity |= min_capacity >> i;
         }

         return ++min_capacity;
      }

   private:
      pointer mp_begin{nullptr};

      alignas(alignof(Any)) std::array<std::byte, sizeof(Any) * Size> m_static_storage;

      size_type m_size{0u};
      size_type m_capacity{0u};

      allocator_type m_allocator;
   };

   template <std::equality_comparable Any, std::size_t SizeOne, std::size_t SizeTwo,
             typename allocator>
   constexpr auto operator==(const basic_dynamic_array<Any, SizeOne, allocator>& lhs,
                             const basic_dynamic_array<Any, SizeTwo, allocator>& rhs) -> bool
   {
      return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
   }

   template <typename Any, std::size_t SizeOne, std::size_t SizeTwo, typename allocator>
   constexpr auto operator<=>(const basic_dynamic_array<Any, SizeOne, allocator>& lhs,
                              const basic_dynamic_array<Any, SizeTwo, allocator>& rhs)
   {
      return std::lexicographical_compare_three_way(std::begin(lhs), std::end(lhs), std::begin(rhs),
                                                    std::end(rhs), detail::synth_three_way);
   }

   template <typename Iter, std::size_t Size = 0,
             typename Allocator =
                std::pmr::polymorphic_allocator<typename std::iterator_traits<Iter>::value_type>>
   basic_dynamic_array(Iter, Iter)
      -> basic_dynamic_array<typename std::iterator_traits<Iter>::value_type, Size, Allocator>;

   template <typename Any, typename... U, typename Allocator = std::pmr::polymorphic_allocator<Any>>
   basic_dynamic_array(Any, U...) -> basic_dynamic_array<Any, 1 + sizeof...(U), Allocator>;

   /**
    * @author wmbat wmbat@protonmail.com
    * @date Sunday, 13th of december 2020
    * @brief A resizable array with a small statically allocated storage buffer
    * @copyright MIT License
    *
    * @tparam Any The type of the elements
    * @tparam Size The size of the staticly allocated small buffer.
    */
   template <typename Any, std::size_t Size>
   class small_dynamic_array
   {
      using underlying_type = basic_dynamic_array<Any, Size, std::pmr::polymorphic_allocator<Any>>;

   public:
      using value_type = Any;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = std::pmr::polymorphic_allocator<Any>;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<pointer>;
      using const_iterator = random_access_iterator<const_pointer>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      /**
       * @brief Default constructor.
       */
      constexpr small_dynamic_array() noexcept(noexcept(underlying_type{})) = default;
      /**
       * @brief Construct the container with count copies of elements with value value
       *
       * @param[in] count The size of the container.
       * @param[in] The value to initialize elements from.
       */
      constexpr small_dynamic_array(size_type count, const_reference value) :
         m_underlying{count, value}
      {}
      /**
       * @brief Construct the container with the contents of the initializer list init.
       *
       * @param[in] init Initializer list to initialize the elements of the container with.
       */
      constexpr small_dynamic_array(std::initializer_list<Any> init) : m_underlying{init} {}
      /**
       * @brief Construct the container with the contents of the range [first, last)
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       */
      template <std::input_iterator InputIt>
      constexpr small_dynamic_array(InputIt first, InputIt last) : m_underlying{first, last}
      {}

      /**
       * @brief Replaces the contents with those identified by initializer list init_list
       *
       * @param init_list Initializer list to use as data source.
       */
      constexpr auto operator=(std::initializer_list<Any> init_list) -> small_dynamic_array&
      {
         m_underlying = init_list;

         return *this;
      }

      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) -> reference { return m_underlying.lookup(index); }
      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A const reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) const -> const_reference
      {
         return m_underlying.lookup(index);
      }

      /**
       * @brief Access the data stored by the container.
       *
       * @return A pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() noexcept -> pointer { return m_underlying.data(); }
      /**
       * @brief Access the data stored by the container.
       *
       * @return A const_pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() const noexcept -> const_pointer { return m_underlying.data(); }

      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return m_underlying.begin(); }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return m_underlying.begin(); }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return iterator to the first element. If the basic_dynamic_array is empty, the
       * const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return m_underlying.cbegin(); }

      /**
       * @brief Get an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() noexcept -> iterator { return m_underlying.end(); }
      /**
       * @brief Return an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() const noexcept -> const_iterator { return m_underlying.end(); }
      /**
       * @brief Returns an it iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto cend() const noexcept -> const_iterator { return m_underlying.cend(); }

      /**
       * @brief Returns a reverse iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rbegin() noexcept -> reverse_iterator { return m_underlying.rbegin(); }
      /**
       * @brief Returns a reverse_iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return reverse_iterator to the first element.
       */
      constexpr auto rbegin() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rbegin();
      }
      /**
       * @brief Returns a reverse iterator to the first element of the reversed
       * basic_dynamic_array. It corresponds to the last element of the non-reversed
       * basic_dynamic_array. If the basic_dynamic_array is empty, the returned iterator is equal to
       * rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rcbegin();
      }

      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() noexcept -> reverse_iterator { return m_underlying.end(); }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() const noexcept -> const_reverse_iterator { return m_underlying.rend(); }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rcend() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rcend();
      }

      /**
       * @brief Check if the basic_dynamic_array is empty.
       *
       * @return True if the container is empty, false otherwise.
       */
      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_underlying.empty(); };
      /**
       * @brief Check the number of elements stored in the basic_dynamic_array.
       *
       * @return The number of elements in the basic_dynamic_array.
       */
      [[nodiscard]] constexpr auto size() const noexcept -> size_type
      {
         return m_underlying.size();
      };
      /**
       * @brief Check the number of elements that the basic_dynamic_array has currently allocated
       * space for.
       *
       * @return Capacity of the currently allocated storage.
       */
      [[nodiscard]] constexpr auto capacity() const noexcept -> size_type
      {
         return m_underlying.capacity();
      };
      /**
       * @brief Increase the capacity of the vector to a value that's greater or equal to new_cap.
       * If new_ap is greater than the current capacity(), new storage is allocated, otherwise the
       * method does nothing. If reallocated occurs, all current iterators are invalidated.
       *
       * @param new_cap New capacity of the vector.
       *
       * @throws If the undelying allocator failed to allocate memory.
       */
      constexpr void reserve(size_type new_cap) { m_underlying.reserve(new_cap); }

      /**
       * @brief Erases all elements from the container, After this call, size() returs zero.
       */
      constexpr void clear() noexcept { m_underlying.clear(); }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, const_reference value) -> iterator
      {
         return m_underlying.insert(pos, value);
      }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, value_type&& value) -> iterator
      {
         return m_underlying.insert(pos, std::move(value));
      }
      /**
       * @brief Insert a new element into the container directly before pos. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * @param[in] args Arguments to forward to the constructor of the element.
       *
       * @return Iterator pointing to the inserted value.
       */
      template <typename... Args>
      constexpr auto insert(const_iterator pos, Args... args) -> iterator
      {
         return m_underlying.insert(pos, std::forward<Args>(args)...);
      }
      /**
       * @brief Inserts count elements from a specified value.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] count The number of elements to insert.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, size_type count, const_reference value) -> iterator
      {
         return m_underlying.insert(pos, count, value);
      }

      /**
       * @brief Inserts elements from a range [first, last) before pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] first The first value to insert
       * @param[in] last One past the last value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      template <std::input_iterator InputIt>
      constexpr auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator
      {
         return m_underlying.insert(pos, first, last);
      }

      /**
       * @brief Insert elements from an initializer_list before the position pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] init_list Initializer list to insert the values from.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, std::initializer_list<value_type> init_list)
         -> iterator
      {
         return m_underlying(pos, init_list);
      }

      /**
       * @brief Erases the specified element from the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator to the element to remove.
       */
      constexpr auto erase(const_iterator pos) -> iterator { return m_underlying.erase(pos); }
      /**
       * @brief Erases the specified elements from the container.
       *
       * @pre first >= begin()
       * @pre last <= end()
       * @pre first >= last
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       *
       * @return Iterator following the last removed element. If last == end() prior to removal,
       * then the updated end() iterator is returned. If [first, last) is an empty range, the last
       * iterator is returned.
       */
      constexpr auto erase(const_iterator first, const_iterator last) -> iterator
      {
         return m_underlying.erase(first, last);
      }

      /**
       * @brief Appends the given element value to the end of the container. The new element is
       * initialized as a copy of value.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(const value_type& value) { m_underlying.append(value); }
      /**
       * @brief Appends the given element value to the end of the container. Value is moved into the
       * new element.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(value_type&& value) { m_underlying.append(std::move(value)); }
      /**
       * @brief Appends the given element value to the end of the container. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @param[in] args Arguments to forward to the constructor of the element.
       */
      template <typename... Args>
      constexpr auto append(Args&&... args) -> reference
      {
         return m_underlying.append(std::forward<Args>(args)...);
      }

      /**
       * @brief Removes the last element in the container.
       *
       * @pre size() != 0
       */
      constexpr void pop_back() { return m_underlying.pop_back(); };

      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, default constructed elements are appended.
       *
       * @param[in] count New size of the container.
       */
      constexpr void resize(size_type count) { m_underlying.resize(count); }
      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, additional copies of value are appended.
       *
       * @param[in] count New size of the container.
       * @param[in] value The value to initialize new elements with.
       */
      constexpr void resize(size_type count, const_reference value)
      {
         m_underlying.resize(count, value);
      }

   private:
      underlying_type m_underlying;
   };

   template <std::equality_comparable Any, std::size_t SizeOne, std::size_t SizeTwo>
   constexpr auto operator==(const small_dynamic_array<Any, SizeOne>& lhs,
                             const small_dynamic_array<Any, SizeTwo>& rhs) -> bool
   {
      return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
   }

   template <typename Any, std::size_t SizeOne, std::size_t SizeTwo>
   constexpr auto operator<=>(const small_dynamic_array<Any, SizeOne>& lhs,
                              const small_dynamic_array<Any, SizeTwo>& rhs)
   {
      return std::lexicographical_compare_three_way(std::begin(lhs), std::end(lhs), std::begin(rhs),
                                                    std::end(rhs), detail::synth_three_way);
   }

   template <typename Iter, std::size_t Size = 0>
   small_dynamic_array(Iter, Iter)
      -> small_dynamic_array<typename std::iterator_traits<Iter>::value_type, Size>;

   template <typename Any, typename... U>
   small_dynamic_array(Any, U...) -> small_dynamic_array<Any, 1 + sizeof...(U)>;

   /**
    * @author wmbat wmbat@protonmail.com
    * @date Sunday, 13th of december 2020
    * @brief A resizable array
    * @copyright MIT License
    *
    * @tparam Any The type of the elements
    */
   template <typename Any>
   class dynamic_array
   {
      using underlying_type = basic_dynamic_array<Any, 0u, std::pmr::polymorphic_allocator<Any>>;

   public:
      using value_type = Any;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = std::pmr::polymorphic_allocator<Any>;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<pointer>;
      using const_iterator = random_access_iterator<const_pointer>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      /**
       * @brief Default constructor.
       */
      constexpr dynamic_array() noexcept(noexcept(underlying_type{})) = default;
      /**
       * @brief Construct the container with count copies of elements with value value
       *
       * @param[in] count The size of the container.
       * @param[in] The value to initialize elements from.
       */
      constexpr dynamic_array(size_type count, const_reference value) : m_underlying{count, value}
      {}
      /**
       * @brief Construct the container with the contents of the initializer list init.
       *
       * @param[in] init Initializer list to initialize the elements of the container with.
       */
      constexpr dynamic_array(std::initializer_list<Any> init) : m_underlying{init} {}
      /**
       * @brief Construct the container with the contents of the range [first, last)
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       */
      template <std::input_iterator InputIt>
      constexpr dynamic_array(InputIt first, InputIt last) : m_underlying{first, last}
      {}

      /**
       * @brief Replaces the contents with those identified by initializer list init_list
       *
       * @param init_list Initializer list to use as data source.
       */
      constexpr auto operator=(std::initializer_list<Any> init_list) -> dynamic_array&
      {
         m_underlying = init_list;

         return *this;
      }

      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) -> reference { return m_underlying.lookup(index); }
      /**
       * @brief Access the object stored at a specific index.
       *
       * @param index The position to lookup the object in the array
       *
       * @pre index must be less than the size.
       *
       * @return A const reference to the object stored at index.
       */
      constexpr auto lookup(size_type index) const -> const_reference
      {
         return m_underlying.lookup(index);
      }

      /**
       * @brief Access the data stored by the container.
       *
       * @return A pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() noexcept -> pointer { return m_underlying.data(); }
      /**
       * @brief Access the data stored by the container.
       *
       * @return A const_pointer to the first element in the container. If no elements are in the
       * container, the pointer will be null.
       */
      constexpr auto data() const noexcept -> const_pointer { return m_underlying.data(); }

      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return m_underlying.begin(); }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return m_underlying.begin(); }
      /**
       * @brief Returns an iterator to the first element of the basic_dynamic_array.
       *
       * @return iterator to the first element. If the basic_dynamic_array is empty, the
       * const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return m_underlying.cbegin(); }

      /**
       * @brief Get an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() noexcept -> iterator { return m_underlying.end(); }
      /**
       * @brief Return an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto end() const noexcept -> const_iterator { return m_underlying.end(); }
      /**
       * @brief Returns an it iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return iterator to the element following the last element. Attempting to access it results
       * in undefined behaviour.
       */
      constexpr auto cend() const noexcept -> const_iterator { return m_underlying.cend(); }

      /**
       * @brief Returns a reverse iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rbegin() noexcept -> reverse_iterator { return m_underlying.rbegin(); }
      /**
       * @brief Returns a reverse_iterator to the first element of the reversed basic_dynamic_array.
       * It corresponds to the last element of the non-reversed basic_dynamic_array. If the
       * basic_dynamic_array is empty, the returned iterator is equal to rend().
       *
       * @return reverse_iterator to the first element.
       */
      constexpr auto rbegin() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rbegin();
      }
      /**
       * @brief Returns a reverse iterator to the first element of the reversed
       * basic_dynamic_array. It corresponds to the last element of the non-reversed
       * basic_dynamic_array. If the basic_dynamic_array is empty, the returned iterator is equal to
       * rend().
       *
       * @return Reverse iterator to the first element.
       */
      constexpr auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rcbegin();
      }

      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() noexcept -> reverse_iterator { return m_underlying.end(); }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rend() const noexcept -> const_reverse_iterator { return m_underlying.rend(); }
      /**
       * @brief Returns a reverse iterator to the element following the last element of the reversed
       * basic_dynamic_array. It corresponds to the element preceding the first element of the
       * non-reversed basic_dynamic_array. This element acts as a placeholder, attempting to access
       * it results in UB.
       *
       * @return Reverse iterator to the element following the last element.
       */
      constexpr auto rcend() const noexcept -> const_reverse_iterator
      {
         return m_underlying.rcend();
      }

      /**
       * @brief Check if the basic_dynamic_array is empty.
       *
       * @return True if the container is empty, false otherwise.
       */
      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_underlying.empty(); };
      /**
       * @brief Check the number of elements stored in the basic_dynamic_array.
       *
       * @return The number of elements in the basic_dynamic_array.
       */
      [[nodiscard]] constexpr auto size() const noexcept -> size_type
      {
         return m_underlying.size();
      };
      /**
       * @brief Check the number of elements that the basic_dynamic_array has currently allocated
       * space for.
       *
       * @return Capacity of the currently allocated storage.
       */
      [[nodiscard]] constexpr auto capacity() const noexcept -> size_type
      {
         return m_underlying.capacity();
      };
      /**
       * @brief Increase the capacity of the vector to a value that's greater or equal to new_cap.
       * If new_ap is greater than the current capacity(), new storage is allocated, otherwise the
       * method does nothing. If reallocated occurs, all current iterators are invalidated.
       *
       * @param new_cap New capacity of the vector.
       *
       * @throws If the undelying allocator failed to allocate memory.
       */
      constexpr void reserve(size_type new_cap) { m_underlying.reserve(new_cap); }

      /**
       * @brief Erases all elements from the container, After this call, size() returs zero.
       */
      constexpr void clear() noexcept { m_underlying.clear(); }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, const_reference value) -> iterator
      {
         return m_underlying.insert(pos, value);
      }

      /**
       * @brief Inserts an element value at the position before pos in the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the inserted value.
       */
      constexpr auto insert(const_iterator pos, value_type&& value) -> iterator
      {
         return m_underlying.insert(pos, std::move(value));
      }
      /**
       * @brief Insert a new element into the container directly before pos. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * @param[in] args Arguments to forward to the constructor of the element.
       *
       * @return Iterator pointing to the inserted value.
       */
      template <typename... Args>
      constexpr auto insert(const_iterator pos, Args... args) -> iterator
      {
         return m_underlying.insert(pos, std::forward<Args>(args)...);
      }
      /**
       * @brief Inserts count elements from a specified value.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] count The number of elements to insert.
       * @param[in] value Element value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, size_type count, const_reference value) -> iterator
      {
         return m_underlying.insert(pos, count, value);
      }

      /**
       * @brief Inserts elements from a range [first, last) before pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] first The first value to insert
       * @param[in] last One past the last value to insert.
       *
       * @return Iterator pointing to the first element inserted.
       */
      template <std::input_iterator InputIt>
      constexpr auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator
      {
         return m_underlying.insert(pos, first, last);
      }

      /**
       * @brief Insert elements from an initializer_list before the position pos.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator before which the content will be inserted. pos may be the end()
       * iterator.
       * @param[in] init_list Initializer list to insert the values from.
       *
       * @return Iterator pointing to the first element inserted.
       */
      constexpr auto insert(const_iterator pos, std::initializer_list<value_type> init_list)
         -> iterator
      {
         return m_underlying(pos, init_list);
      }

      /**
       * @brief Erases the specified element from the container.
       *
       * @pre pos >= begin()
       * @pre pos <= end()
       *
       * @param[in] pos Iterator to the element to remove.
       */
      constexpr auto erase(const_iterator pos) -> iterator { return m_underlying.erase(pos); }
      /**
       * @brief Erases the specified elements from the container.
       *
       * @pre first >= begin()
       * @pre last <= end()
       * @pre first >= last
       *
       * @param[in] first The first element of the range to copy from.
       * @param[in] last One past the last element of the range to copy from.
       *
       * @return Iterator following the last removed element. If last == end() prior to removal,
       * then the updated end() iterator is returned. If [first, last) is an empty range, the last
       * iterator is returned.
       */
      constexpr auto erase(const_iterator first, const_iterator last) -> iterator
      {
         return m_underlying.erase(first, last);
      }

      /**
       * @brief Appends the given element value to the end of the container. The new element is
       * initialized as a copy of value.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(const value_type& value) { m_underlying.append(value); }
      /**
       * @brief Appends the given element value to the end of the container. Value is moved into the
       * new element.
       *
       * @param[in] value The value of the element to append.
       */
      constexpr void append(value_type&& value) { m_underlying.append(std::move(value)); }
      /**
       * @brief Appends the given element value to the end of the container. The element is
       * constructed in-place using the arguments Args... that are forwarded to the constructor.
       *
       * @param[in] args Arguments to forward to the constructor of the element.
       */
      template <typename... Args>
      constexpr auto append(Args&&... args) -> reference
      {
         return m_underlying.append(std::forward<Args>(args)...);
      }

      /**
       * @brief Removes the last element in the container.
       *
       * @pre size() != 0
       */
      constexpr void pop_back() { return m_underlying.pop_back(); };

      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, default constructed elements are appended.
       *
       * @param[in] count New size of the container.
       */
      constexpr void resize(size_type count) { m_underlying.resize(count); }
      /**
       * @brief Resizes the container to contain count elements. If the current size is greater than
       * count, the container is reduced to its first count elements. If the current size is less
       * than count, additional copies of value are appended.
       *
       * @param[in] count New size of the container.
       * @param[in] value The value to initialize new elements with.
       */
      constexpr void resize(size_type count, const_reference value)
      {
         m_underlying.resize(count, value);
      }

   private:
      underlying_type m_underlying;
   };

   template <std::equality_comparable Any>
   constexpr auto operator==(const dynamic_array<Any>& lhs, const dynamic_array<Any>& rhs) -> bool
   {
      return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
   }

   template <typename Any>
   constexpr auto operator<=>(const dynamic_array<Any>& lhs, const dynamic_array<Any>& rhs)
   {
      return std::lexicographical_compare_three_way(std::begin(lhs), std::end(lhs), std::begin(rhs),
                                                    std::end(rhs), detail::synth_three_way);
   }

} // namespace crl

namespace std // NOLINT
{
   /**
    * @brief Erases all elements that compare equal to value from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] value Value to be removed.
    *
    * @return The number of erased elements.
    */
   template <typename Any, std::size_t Size, typename Allocator, typename Value>
   constexpr auto erase(crl::basic_dynamic_array<Any, Size, Allocator>& arr, const Value& value)
   {
      const auto it = std::remove(std::begin(arr), std::end(arr), value);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }

   /**
    * @brief Erases all elements that satisfy the predicate pred from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] pred Unary predicate which returns `true` if the element should be erased.
    *
    * @return The number of erased elements.
    */
   template <typename Any, std::size_t Size, typename Allocator, typename Pred>
   constexpr auto erase_if(crl::basic_dynamic_array<Any, Size, Allocator>& arr, Pred pred)
   {
      const auto it = std::remove_if(std::begin(arr), std::end(arr), pred);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }

   /**
    * @brief Erases all elements that compare equal to value from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] value Value to be removed.
    *
    * @return The number of erased elements.
    */
   template <typename Any, std::size_t Size, typename Value>
   constexpr auto erase(crl::small_dynamic_array<Any, Size>& arr, const Value& value)
   {
      const auto it = std::remove(std::begin(arr), std::end(arr), value);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }

   /**
    * @brief Erases all elements that satisfy the predicate pred from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] pred Unary predicate which returns `true` if the element should be erased.
    *
    * @return The number of erased elements.
    */
   template <typename Any, std::size_t Size, typename Pred>
   constexpr auto erase_if(crl::small_dynamic_array<Any, Size>& arr, Pred pred)
   {
      const auto it = std::remove_if(std::begin(arr), std::end(arr), pred);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }

   /**
    * @brief Erases all elements that compare equal to value from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] value Value to be removed.
    *
    * @return The number of erased elements.
    */
   template <typename Any, typename Value>
   constexpr auto erase(crl::dynamic_array<Any>& arr, const Value& value)
   {
      const auto it = std::remove(std::begin(arr), std::end(arr), value);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }

   /**
    * @brief Erases all elements that satisfy the predicate pred from the container.
    *
    * @param[in] arr Container from which to erase.
    * @param[in] pred Unary predicate which returns `true` if the element should be erased.
    *
    * @return The number of erased elements.
    */
   template <typename Any, typename Pred>
   constexpr auto erase_if(crl::dynamic_array<Any>& arr, Pred pred)
   {
      const auto it = std::remove_if(std::begin(arr), std::end(arr), pred);
      const auto r = std::distance(it, std::end(arr));
      arr.erase(it, std::end(arr));
      return r;
   }
} // namespace std
