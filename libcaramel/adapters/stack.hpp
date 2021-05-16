#pragma once

#include <libcaramel/containers/concepts.hpp>
#include <libcaramel/containers/deque.hpp>

namespace caramel
{
   template <typename Any, sequence_container Container = deque<Any>>
   class basic_stack
   {
   public:
   private:
      deque<Any> m_underlying;
   };

   template <typename Any>
   class stack
   {
   public:
   private:
      stack<Any> m_underlying;
   };
} // namespace caramel
