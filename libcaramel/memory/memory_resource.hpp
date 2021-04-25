/**
 * @file memory/memory_resource.hpp
 * @author wmbat wmbat@protonmail.com
 * @brief
 * @date Saturday, 24th of April 2021
 * @copyright Copyright (C) 2021 wmbat.
 */

#pragma once

#include <libcaramel/util/types.hpp>

#include <gsl/pointers>

#include <cstddef>

namespace caramel
{
   /**
    * @brief Abstract class defining the interface of a memory resource
    */
   class memory_resource
   {
   public:
      using pointer = void*;             ///< alias for ease of naming
      using const_pointer = const void*; ///< alias for ease of naming

   public:
      memory_resource() noexcept = default;
      memory_resource(const memory_resource&) noexcept = default;
      memory_resource(memory_resource&&) noexcept = default;
      virtual ~memory_resource() noexcept = default;

      auto operator=(const memory_resource&) noexcept -> memory_resource& = default;
      auto operator=(memory_resource&&) noexcept -> memory_resource& = default;

      /**
       * @brief Check if two memory_resources are equal
       * @details Use the virtual is_equal function to compare if two memory_resources are equal
       * like so: `this == &rhs or is_equal(rhs)`.
       *
       * @param[in] other
       *
       * @return
       */
      auto operator==(const memory_resource& rhs) const -> bool;

      /**
       * @brief Pure virtual function for a common allocation interface.
       *
       * @pre `bytes >= 0`, otherwise UB
       * @pre `alignment >= 0`, otherwise UB
       *
       * @param[in] bytes The size of the allocation in bytes
       * @param[in] alignment The alignment of the allocation in bytes
       *
       * @return A valid pointer to a memory chunk or nullptr if the allocation failed
       */
      virtual auto allocate(count_t bytes, align_t alignment) noexcept -> pointer = 0;
      /**
       * @brief Pure virtual function for a common deallocation interface.
       *
       * @pre `bytes >= 0`, otherwise UB
       * @pre `alignment >= 0`, otherwise UB
       *
       * @param[in] ptr The starting location of the memory chunk
       * @param[in] bytes The size of the allocation in bytes
       * @param[in] alignment The alignment of the allocation in bytes
       */
      virtual void deallocate(gsl::not_null<pointer> ptr, count_t bytes,
                              align_t alignment) noexcept = 0;
      /**
       * @brief Pure virtual function for a common comparison interface.
       *
       * @param[in] other The memory_resource to compare with.
       *
       * @return True if both memory_resources are considered equal, otherwise false
       */
      virtual auto is_equal(const memory_resource& other) const noexcept -> bool = 0;
   };

   /**
    * @brief Access the default memory_resource
    */
   auto get_default_memory_resource() noexcept -> memory_resource*;
   /**
    * @brief Set the default memory_resource
    */
   void set_default_memory_resource(gsl::not_null<memory_resource*> p_resource) noexcept;
} // namespace caramel
