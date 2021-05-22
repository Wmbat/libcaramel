#ifndef LIBCARAMEL_ADAPTORS_STACK_HPP
#define LIBCARAMEL_ADAPTORS_STACK_HPP

#include <libcaramel/containers/chained_group.hpp>
#include <libcaramel/containers/concepts.hpp>

namespace caramel
{
   template <typename Any, sequence_container Container = chained_group<Any>>
   class basic_stack
   {
   public:
   private:
      Container m_underlying{};
   };

   template <typename Any>
   class stack
   {
   public:
   private:
      basic_stack<Any> m_underlying;
   };
} // namespace caramel

#endif // LIBCARAMEL_ADAPTORS_STACK_HPP
