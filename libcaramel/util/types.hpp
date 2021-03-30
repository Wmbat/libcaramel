#pragma once

#include <libcaramel/util/strong_type.hpp>

#include <cstdint>

namespace caramel
{
   using size_t = std::int64_t;
   
   using size = strong_type<size_t, struct size_type, comparable>;
} // namespace caramel
