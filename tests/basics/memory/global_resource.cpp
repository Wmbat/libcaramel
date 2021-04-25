#include "gsl/pointers"
#include <doctest/doctest.h>

#include <libcaramel/memory/global_resource.hpp>

#include <memory>

using namespace caramel;

TEST_SUITE("global_resource test suite")
{
   TEST_CASE("i don't even know")
   {
      global_resource g;
      int* ptr = static_cast<int*>(g.allocate(count_t{sizeof(int)}, align_t{alignof(int)}));

      std::construct_at(ptr, 10); // NOLINT

      REQUIRE(*ptr == 10);

      g.deallocate(gsl::make_not_null(static_cast<void*>(ptr)), count_t{sizeof(int)},
                   align_t{alignof(int)});
   }
}
