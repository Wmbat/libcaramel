#include <libcaramel/memory/global_resource.hpp>

#include <gsl/gsl_assert>

#include <new>

namespace caramel
{
   auto global_resource::allocate(count_t bytes, align_t alignment) noexcept -> pointer
   {
      Expects(bytes.value() >= 0);
      Expects(alignment.value() >= 0);

      return static_cast<pointer>(::operator new(static_cast<std::size_t>(bytes.value()),
                                                 static_cast<std::align_val_t>(alignment.value()),
                                                 std::nothrow));
   }
   void global_resource::deallocate(gsl::not_null<pointer> ptr, count_t bytes,
                                    align_t alignment) noexcept
   {
      Expects(bytes.value() >= 0);
      Expects(alignment.value() >= 0);

      ::operator delete(static_cast<pointer>(ptr.get()), static_cast<std::size_t>(bytes.value()),
                        static_cast<std::align_val_t>(alignment.value()));
   }
   auto global_resource::is_equal(const memory_resource& other) const noexcept -> bool
   {
      return this == &other;
   }
} // namespace caramel
