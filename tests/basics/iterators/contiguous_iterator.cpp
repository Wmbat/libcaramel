#include <doctest/doctest.h>

#include <libcaramel/iterators/contiguous_iterator.hpp>

#include <span>

using namespace caramel;

TEST_SUITE("contiguous_iterator") // NOLINT
{
   TEST_CASE("construction")
   {
      SUBCASE("default")
      {
         auto it = contiguous_iterator<int>();

         CHECK(it == nullptr);
      }
      SUBCASE("with pointer")
      {
         int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
         size_t arr_size = sizeof(arr) / sizeof(arr[0]);
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(*beg == arr[0]);
         CHECK(*beg == 1);
         CHECK(*(end - 1) == arr[arr_size - 1]);
         CHECK(*(end - 1) == 6);
      }
   }
   TEST_CASE("iteration")
   {
      SUBCASE("forward")
      {
         int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
         size_t arr_size = sizeof(arr) / sizeof(arr[0]);
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         for (int val = 1; beg != end; ++beg, ++val)
         {
            CHECK(*beg == val);
         }
      }
      SUBCASE("std::span for_each")
      {
         int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
         size_t arr_size = sizeof(arr) / sizeof(arr[0]);
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         auto arr_view = std::span<int>(beg, end);

         int val = 1;
         for (int i : arr_view)
         {
            CHECK(i == val++);
         }
      }
   }
}

