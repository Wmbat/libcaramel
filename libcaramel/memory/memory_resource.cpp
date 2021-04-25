#include <libcaramel/memory/memory_resource.hpp>

#include <libcaramel/memory/global_resource.hpp>

namespace caramel
{
   global_resource default_resource{};                      // NOLINT
   memory_resource* p_default_resource = &default_resource; // NOLINT

   auto memory_resource::operator==(const memory_resource& rhs) const -> bool
   {
      return this == &rhs or is_equal(rhs);
   }

   auto get_default_memory_resource() noexcept -> memory_resource* { return p_default_resource; }
   void set_default_memory_resource(gsl::not_null<memory_resource*> p_resource) noexcept
   {
      p_default_resource = p_resource.get();
   }
} // namespace caramel
