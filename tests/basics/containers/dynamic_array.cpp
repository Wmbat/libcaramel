#include <doctest/doctest.h>

#include <libcaramel/containers/dynamic_array.hpp>

using namespace caramel;

class simple_class
{
public:
   simple_class() = default;
   simple_class(int a, int b) : m_a{a}, m_b{b} {}

   auto a() -> int { return m_a; }
   auto b() -> int { return m_b; }

private:
   int m_a = 0;
   int m_b = 0;
};

TEST_SUITE("dynamic_array test suite")
{
   TEST_CASE("append in place")
   {
      dynamic_array<simple_class> test;

      auto& temp = test.append(in_place, 10, 20);
      CHECK(temp.a() == 10);
      CHECK(temp.b() == 20);
      CHECK(std::size(test) == 1);
      CHECK(test.lookup(0).a() == 10);
      CHECK(test.lookup(0).b() == 20);
   }
}
