#pragma once

#include <libcaramel/util/strong_type.hpp>

#include <cstdint>

namespace caramel
{
   using i32_t = std::int32_t;
   using u32_t = std::uint32_t;
   using i64_t = std::int64_t;
   using u64_t = std::uint64_t;

   using size_t = strong_type<i64_t, struct size_type, comparable>;
   using count_t = strong_type<i64_t, struct count_type, arithmetic>;
   using align_t = strong_type<i64_t, struct align_type, arithmetic>;
} // namespace caramel
