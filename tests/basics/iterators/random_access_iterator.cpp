#include <doctest/doctest.h>

#include <libcaramel/iterators/random_iterator.hpp>

#include <span>

using namespace caramel;

TEST_SUITE("random_access_iterator") // NOLINT
{
   TEST_CASE("construction")
   {
      SUBCASE("default")
      {
         auto it = random_access_iterator<int>();

         CHECK(it == nullptr);
      }
      SUBCASE("with pointer")
      {
         int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
         size_t arr_size = sizeof(arr) / sizeof(arr[0]);
         auto beg = random_access_iterator<int>(&arr[0]);
         auto end = random_access_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(*beg == arr[0]);
         CHECK(*(end - 1) == arr[arr_size - 1]);
      }
   }
   TEST_CASE("iteration")
   {
      SUBCASE("forward")
      {
         int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
         size_t arr_size = sizeof(arr) / sizeof(arr[0]);
         auto beg = random_access_iterator<int>(&arr[0]);
         auto end = random_access_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(*beg == arr[0]);
         CHECK(*(end - 1) == arr[arr_size - 1]); // NOLINT

         for (int val = 1; beg != end; ++beg, ++val)
         {
            CHECK(*beg == val);
         }
      }
      SUBCASE("backward") {}
   }
}
