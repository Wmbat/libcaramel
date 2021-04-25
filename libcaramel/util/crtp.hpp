#pragma once

namespace caramel
{
   template <typename any_, template <typename> class crtp_type_>
   struct crtp
   {
      constexpr auto underlying() -> any_& { return static_cast<any_&>(*this); }
      constexpr auto underlying() const -> any_ const& { return static_cast<any_ const&>(*this); }
   };
} // namespace caramel
