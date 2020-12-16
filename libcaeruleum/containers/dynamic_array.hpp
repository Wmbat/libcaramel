#pragma once

#include <libcaeruleum/assert.hpp>
#include <libcaeruleum/iterators/random_access.hpp>

#include <monads/result.hpp>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <memory_resource>
#include <type_traits>

namespace crl
{
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
      using value_type = Any; /**< The type of the elements store in the basic_dynamic_array */
      using size_type = std::size_t;          /**< unsigned integer type */
      using difference_type = std::ptrdiff_t; /**< signed integer type */
      using allocator_type = Allocator;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<value_type, false>;
      using const_iterator = random_access_iterator<value_type, true>;
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

      constexpr auto operator=(const basic_dynamic_array& rhs) -> basic_dynamic_array&
      {
         if (this != &rhs)
         {
            copy_assign_alloc(rhs);
            assign(rhs.begin(), rhs.end());
         }

         return *this;
      }

      constexpr auto operator=(basic_dynamic_array&& rhs) noexcept(
         allocator_type::propagate_on_container_move_assignment::value ||
         allocator_type::is_always_equal::value) -> basic_dynamic_array&
      {
         move_assign(rhs,
                     std::integral_constant<
                        bool, allocator_traits::propagate_on_container_move_assignment::value>());

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
       * @brief Get an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return iterator{mp_begin}; }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return const_iterator{mp_begin}; }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return const_iterator{mp_begin}; }

      /**
       * @brief Get an iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return An iterator to the element following the last element of the basic_dynamic_array.
       * Attempting to access it results in undefined behaviour.
       */
      constexpr auto end() noexcept -> iterator { return iterator{mp_begin + m_size}; }
      /**
       * @brief Get a const_iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return A const_iterator to the element following the last element of the
       * basic_dynamic_array. Attempting to access it results in undefined behaviour.
       */
      constexpr auto end() const noexcept -> const_iterator
      {
         return const_iterator{mp_begin + m_size};
      }
      /**
       * @brief Get a const_iterator to the element following the last element of the
       * basic_dynamic_array.
       *
       * @return A const_iterator to the element following the last element of the
       * basic_dynamic_array. Attempting to access it results in undefined behaviour.
       */
      constexpr auto cend() const noexcept -> const_iterator
      {
         return const_iterator{mp_begin + m_size};
      }

      constexpr auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }
      constexpr auto rbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }
      constexpr auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }

      constexpr auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }
      constexpr auto rend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }
      constexpr auto rcend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }

      /**
       * @brief Check if the basic_dynamic_array is empty.
       *
       * @return True if the container is empty, false otherwise.
       */
      [[nodiscard]] constexpr auto is_empty() const noexcept -> bool { return begin() == end(); };
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
      constexpr void reserve(size_type new_cap);

      constexpr void clear() noexcept
      {
         destroy(begin(), end());
         m_size = 0;
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

      constexpr void move_assign_alloc(const basic_dynamic_array& other) noexcept(
         std::is_nothrow_assignable_v<allocator_type>)
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
            mp_begin = other.m_pbegin;
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

         const size_type new_count = std::distance(first, last);
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
      using value_type = Any; /**< The type of the elements store in the basic_dynamic_array */
      using size_type = std::size_t;          /**< unsigned integer type */
      using difference_type = std::ptrdiff_t; /**< signed integer type */
      using allocator_type = std::pmr::polymorphic_allocator<Any>;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<value_type, false>;
      using const_iterator = random_access_iterator<value_type, true>;
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
       * @brief Get an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return m_underlying.begin(); }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return m_underlying.begin(); }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return m_underlying.cbegin(); }

   private:
      underlying_type m_underlying;
   };

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
      using value_type = Any; /**< The type of the elements store in the basic_dynamic_array */
      using size_type = std::size_t;          /**< unsigned integer type */
      using difference_type = std::ptrdiff_t; /**< signed integer type */
      using allocator_type = std::pmr::polymorphic_allocator<Any>;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<value_type, false>;
      using const_iterator = random_access_iterator<value_type, true>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
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
       * @brief Get an iterator to the first element of the basic_dynamic_array.
       *
       * @return An iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the iterator will be equal to end().
       */
      constexpr auto begin() noexcept -> iterator { return m_underlying.begin(); }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto begin() const noexcept -> const_iterator { return m_underlying.begin(); }
      /**
       * @brief Get an const_iterator to the first element of the basic_dynamic_array.
       *
       * @return A const_iterator to the first element of the basic_dynamic_array. If the
       * basic_dynamic_array is empty, the const_iterator will be equal to end().
       */
      constexpr auto cbegin() const noexcept -> const_iterator { return m_underlying.cbegin(); }

   private:
      basic_dynamic_array<Any, 0u, std::pmr::polymorphic_allocator<Any>> m_underlying;
   };
} // namespace crl
