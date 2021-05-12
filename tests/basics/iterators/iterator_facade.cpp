#include <doctest/doctest.h>

#include <iterator>
#include <libcaramel/iterators/iterator_facade.hpp>
#include <libcaramel/iterators/random_iterator.hpp>
#include <type_traits>
#include <vector>

using namespace caramel;

enum class month : int
{
   january,
   february,
   march,
   april,
   may,
   june,
   july,
   august,
   september,
   october,
   november,
   december,
};

class month_iterator : public iterator_facade<month_iterator>
{
   month m_cur = month::january; // Begin at January

public:
   month_iterator() = default;
   explicit month_iterator(month m) : m_cur(m) {}

   // Become a range of months:
   [[nodiscard]] auto begin() const { return *this; }
   [[nodiscard]] auto end() const
   {
      // Return a "month after december" to represent the end
      auto after_december = int(month::december) + 1;
      return month_iterator(month(after_december));
   }

   // Three minimum-required APIs
   auto dereference() const -> const month& { return m_cur; } // NOLINT
   void increment() { m_cur = month(int(m_cur) + 1); }
   void decrement() { m_cur = month(int(m_cur) - 1); }
   auto equal_to(month_iterator o) const -> bool { return m_cur == o.m_cur; } // NOLINT
};

class iota_iterator : public iterator_facade<iota_iterator>
{
   int _value = 0;

public:
   iota_iterator() = default;
   explicit iota_iterator(int i) : _value(i) {}

   [[nodiscard]] auto dereference() const noexcept -> int { return _value; }

   void advance(int off) noexcept { _value += off; }
   [[nodiscard]] auto distance_to(iota_iterator o) const noexcept -> int { return *o - **this; }
   auto operator==(iota_iterator o) const noexcept -> bool { return *o == **this; }
};

TEST_SUITE("iterator_facade test suite") // NOLINT
{
   TEST_CASE("Create an iota_iterator") // NOLINT
   {
      iota_iterator it;
      iota_iterator stop{44}; // NOLINT
      CHECK((stop - it) == 44);

      CHECK(it < stop);
      CHECK(it <= stop);
      CHECK_FALSE(it > stop);
      CHECK_FALSE(it >= stop);

      CHECK(std::distance(it, stop) == 44);

      CHECK(it[33] == 33);
      CHECK(it[-9] == -9);
      CHECK(stop[2] == 46);
      CHECK(stop[-44] == 0);

      CHECK((stop - it) == 44);
      CHECK((it - stop) == -44);

      CHECK(it != stop);
      CHECK((it + 44) == stop);
      CHECK(it == (stop - 44));
   }

   TEST_CASE("arrow_proxy")
   {
      detail::arrow_proxy<std::string> s{""};
      s->append("Hello, ");
      s->append("world!");
      CHECK(*s.operator->() == "Hello, world!");
   }

   TEST_CASE("Trivial iterator")
   {
      struct deref_iter : iterator_facade<deref_iter>
      {
         int* value = nullptr;
         [[nodiscard]] auto dereference() const noexcept -> int& { return *value; }

         deref_iter(int& i) : value(&i) {}
      };

      // Not even an increment operator.

      int i = 12;
      deref_iter it{i};

      CHECK(*it == 12);
      i = 7;
      CHECK(*it == 7);
   }

   TEST_CASE("Single-pass iterator")
   {
      struct in_iter : iterator_facade<in_iter>
      {
         int value = 0;
         enum
         {
            single_pass_iterator = true
         };

         const int& dereference() const noexcept { return value; }
         void increment() noexcept { ++value; }
      };

      in_iter it;
      CHECK(*it == 0);
      static_assert(std::is_same_v<decltype(++it), in_iter&>);
      ++it;
      CHECK(*it == 1);
      static_assert(std::is_void_v<decltype(it++)>);
   }

   TEST_CASE("Sentinel support")
   {
      struct until_7_iter : iterator_facade<until_7_iter>
      {
         int value = 0;
         struct sentinel_type
         {
         };

         auto dereference() const noexcept { return value; }
         auto increment() noexcept { ++value; }

         auto distance_to(sentinel_type) const noexcept { return 7 - value; }
         bool operator==(sentinel_type s) const noexcept { return distance_to(s) == 0; }
      };

      struct seven_range
      {
         auto begin() { return until_7_iter(); }
         auto end() { return until_7_iter::sentinel_type(); }
      };

      int sum = 0;
      for (auto i : seven_range())
      {
         sum += i;
         CHECK(i < 7);
      }
      CHECK(sum == (1 + 2 + 3 + 4 + 5 + 6));

      auto it = seven_range().begin();
      auto stop = seven_range().end();
      CHECK(it != stop);
      CHECK(stop != it);
      CHECK_FALSE(it == stop);
      CHECK_FALSE(stop == it);
   }
}
