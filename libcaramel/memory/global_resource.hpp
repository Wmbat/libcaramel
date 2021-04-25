/**
 * @file memory/global_resource.hpp
 * @author wmbat wmbat@protonmail.com
 * @brief
 * @date Saturday, 24th of April 2021
 * @copyright Copyright (C) 2021 wmbat.
 */

#pragma once

#include <libcaramel/memory/memory_resource.hpp>

namespace caramel
{
   /**
    * @brief Simple allocator for global allocations
    */
   class global_resource : public memory_resource
   {
   public:
      using pointer = typename memory_resource::pointer;
      using const_pointer = typename memory_resource::const_pointer;

   public:
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
      auto allocate(count_t bytes, align_t alignment) noexcept -> pointer override;
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
      void deallocate(gsl::not_null<pointer> ptr, count_t bytes,
                      align_t alignment) noexcept override;
      /**
       * @brief Pure virtual function for a common comparison interface.
       *
       * @param[in] other The memory_resource to compare with.
       *
       * @return True if both memory_resources are considered equal, otherwise false
       */
      auto is_equal(const memory_resource& other) const noexcept -> bool override;
   };
} // namespace caramel
