#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <libcaramel/caramel.hpp>
#include <libcaramel/containers/dynamic_array.hpp>
#include <libcaramel/version.hpp>

TEST_SUITE("dynamic_array test suite")
{
   using namespace crl;

   TEST_CASE("index based lookup")
   {
      SUBCASE("in range index") { basic_dynamic_array<std::string, 0u> my_str_array{}; }

      SUBCASE("out of range index") { REQUIRE(false); }
   }

   TEST_CASE("iterator based lookup")
   {
      SUBCASE("in range index") { REQUIRE(false); }

      SUBCASE("out of range index") { REQUIRE(false); }
   }
}
