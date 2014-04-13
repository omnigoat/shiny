#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
//======================================================================
namespace dust
{
	struct resource_t : atma::ref_counted
	{
		resource_t(context_ptr const& ctx, resource_usage_flags_t usage_flags)
			: context_(ctx), usage_flags_(usage_flags)
		{}

		virtual ~resource_t()
		{}

		auto context() const -> context_ptr const& { return context_; }
		auto usage_flags() const -> resource_usage_flags_t { return usage_flags_; }

		//auto create_view()

	private:
		context_ptr context_;
		resource_usage_flags_t usage_flags_;
	};
}
