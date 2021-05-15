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
      int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
      size_t arr_size = sizeof(arr) / sizeof(arr[0]);

      SUBCASE("forward")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         for (int val = 1; beg != end; ++beg, ++val)
         {
            CHECK(*beg == val);
         }
      }
      SUBCASE("backward")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         for (int val = 6; beg != end; --end, --val)
         {
            CHECK(*(end - 1) == val);
         }
      }
      SUBCASE("std::span for_each")
      {
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
   TEST_CASE("offset")
   {
      int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
      size_t arr_size = sizeof(arr) / sizeof(arr[0]);

      SUBCASE("addition")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(beg + arr_size == end);
         CHECK(*(beg + 2) == 3);
         CHECK(*(beg + 3) == 4);
         CHECK(*(beg + 4) == 5);
      }
      SUBCASE("substraction")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(end - arr_size == beg);
         CHECK(*(end - 1) == 6);
         CHECK(*(end - 2) == 5);
         CHECK(*(end - 3) == 4);
         CHECK(*(end - 4) == 3);
         CHECK(*(end - 5) == 2);
         CHECK(*(end - 6) == 1);
      }
      SUBCASE("Self addition")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         CHECK(*beg == 1);
         beg += 3;
         CHECK(*beg == 4);
         beg += 2;
         CHECK(*beg == 6);
         beg += 1;
         CHECK(beg == end);
      }
      SUBCASE("Self substraction")
      {
         auto beg = contiguous_iterator<int>(&arr[0]);
         auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

         end -= 1;
         CHECK(*end == 6);
         end -= 2;
         CHECK(*end == 4);
         end -= 3;
         CHECK(*end == 1);

         CHECK(beg == end);
      }
   }
   TEST_CASE("random access")
   {
      int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
      size_t arr_size = sizeof(arr) / sizeof(arr[0]);

      auto beg = contiguous_iterator<int>(&arr[0]);

      CHECK(beg[0] == 1);
      CHECK(beg[1] == 2);
      CHECK(beg[2] == 3);
      CHECK(beg[3] == 4);
      CHECK(beg[4] == 5);
      CHECK(beg[5] == 6);
   }
   TEST_CASE("equality")
   {
      int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
      size_t arr_size = sizeof(arr) / sizeof(arr[0]);

      auto beg = contiguous_iterator<int>(&arr[0]);
      auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

      CHECK(beg != end);
      CHECK(beg + 3 == end - 3);
      CHECK(beg + 2 != end - 3);
   }
   TEST_CASE("comparison")
   {
      int arr[] = {1, 2, 3, 4, 5, 6}; // NOLINT
      size_t arr_size = sizeof(arr) / sizeof(arr[0]);

      auto beg = contiguous_iterator<int>(&arr[0]);
      auto end = contiguous_iterator<int>(&arr[arr_size]); // NOLINT

      CHECK(beg < end);
      CHECK(end > beg);
   }
}
