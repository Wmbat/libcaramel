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
      SUBCASE("in range index") {}

      SUBCASE("out of range index") {}
   }

   TEST_CASE("iterator based lookup")
   {
      SUBCASE("in range index") {}

      SUBCASE("out of range index") {}
   }
}
