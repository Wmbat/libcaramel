/**
 * @file util/in_place.hpp
 * @brief Contains the in_place_t helper struct.
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCARAMEL_UTIL_IN_PLACE_HPP
#define LIBCARAMEL_UTIL_IN_PLACE_HPP

namespace caramel
{
   /**
    * @brief Simple struct used to signify in place construction of an type.
    */
   struct in_place_t
   {
      explicit constexpr in_place_t() = default;
   };

   inline constexpr in_place_t in_place; ///< Instance of in_place_t for ease of use.
} // namespace caramel

#endif // LIBCARAMEL_UTIL_IN_PLACE_HPP
