#include <doctest/doctest.h>

#include <libcaramel/containers/dynamic_array.hpp>

#include <compare>

using namespace caramel;

class simple_class
{
public:
   simple_class() = default;
   simple_class(int a, int b) : m_a{a}, m_b{b} {}

   auto operator==(const simple_class& other) const noexcept -> bool = default;

   auto a() -> int { return m_a; }
   auto b() -> int { return m_b; }

private:
   int m_a = 0;
   int m_b = 0;
};

TEST_SUITE("dynamic_array test suite") // NOLINT
{
   TEST_CASE("default ctor") // NOLINT
   {
      SUBCASE("copyable")
      {
         dynamic_array<simple_class> arr;

         REQUIRE(std::empty(arr));
         REQUIRE(std::size(arr) == 0);
         REQUIRE(std::begin(arr) == std::end(arr));

         REQUIRE(arr.allocator().resource() == get_default_memory_resource());
      }

      SUBCASE("moveable")
      {
         dynamic_array<std::unique_ptr<int>> arr;

         REQUIRE(std::empty(arr));
         REQUIRE(std::size(arr) == 0);
         REQUIRE(std::begin(arr) == std::end(arr));

         REQUIRE(arr.allocator().resource() == get_default_memory_resource());
      }
   }

   TEST_CASE("size-copy ctor") // NOLINT
   {
      simple_class def{10, 20}; // NOLINT

      dynamic_array<simple_class> arr{10, def}; // NOLINT

      REQUIRE(std::size(arr) == 10);
      REQUIRE(arr.allocator().resource() == get_default_memory_resource());

      for (const auto& val : arr)
      {
         CHECK(val == def);
      }
   }

   TEST_CASE("append in place") // NOLINT
   {
      dynamic_array<simple_class> test;

      auto& temp = test.append(in_place, 10, 20); // NOLINT
      CHECK(temp.a() == 10);
      CHECK(temp.b() == 20);
      CHECK(std::size(test) == 1);
      CHECK(test.lookup(0).a() == 10);
      CHECK(test.lookup(0).b() == 20);
   }
}
