/**
 * @file containers/concepts.hpp
 * @brief Contains the concepts defining common APIs for containers.
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCARAMEL_CONTAINERS_CONCEPTS_HPP
#define LIBCARAMEL_CONTAINERS_CONCEPTS_HPP

#include <concepts>
#include <utility>

namespace caramel
{
   // clang-format off
  
   /**
    * @brief Defines the minimum API for containers
    */
   template <typename Any>
   concept container = std::default_initializable<Any>
      and std::move_constructible<Any>
      and std::copy_constructible<Any>
      and std::equality_comparable<Any>
      and requires(Any val) 
      {
         typename Any::value_type;
         typename Any::size_type;
         typename Any::difference_type;
         typename Any::reference;
         typename Any::const_reference;
         typename Any::iterator;
         typename Any::const_iterator;

         { val.size() } -> std::same_as<typename Any::size_type>;
         { val.empty() } -> std::convertible_to<bool>;

         { val.begin() } -> std::same_as<typename Any::iterator>;
         { val.end() } -> std::same_as<typename Any::iterator>;
         { val.cbegin() } -> std::same_as<typename Any::const_iterator>;
         { val.cend() } -> std::same_as<typename Any::const_iterator>;
      };

   /**
    * @brief Defines the minimum API for sequence containers.
    */
   template <typename Any>
   concept sequence_container = container<Any>
      and std::constructible_from<Any, typename Any::size_type, typename Any::const_reference>
      and std::constructible_from<Any, std::initializer_list<typename Any::value_type>>
      and requires(
         Any& c,
         const Any& cc,
         typename Any::value_type val, 
         typename Any::size_type n, 
         typename Any::const_iterator it,
         std::initializer_list<decltype(val)> il) 
      {
         { c.lookup(n) } -> std::same_as<typename Any::reference>;
         { cc.lookup(n) } -> std::same_as<typename Any::const_reference>;

         { c.clear() } -> std::same_as<void>;

         { c.insert(it, val) } -> std::same_as<typename Any::iterator>;
         { c.insert(it, std::move(val)) } -> std::same_as<typename Any::iterator>;
         { c.insert(it, n, val) } -> std::same_as<typename Any::iterator>;
         { c.insert(it, il) } -> std::same_as<typename Any::iterator>;

         { c.erase(it) } -> std::same_as<typename Any::iterator>;
         { c.erase(it, it) } -> std::same_as<typename Any::iterator>;
      };

   // clang-format on
} // namespace caramel

#endif // LIBCARAMEL_CONTAINERS_CONCEPTS_HPP
