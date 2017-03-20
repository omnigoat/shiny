#pragma once

#include <atma/config/platform.hpp>


namespace shiny::api_bridge
{
	template <typename C, typename I>
	inline constexpr C* inteface_to_concrete(I* i)
	{
		return reinterpret_cast<C*>((char*)i + sizeof(I));
	}

	template <typename I, typename C>
	inline constexpr C* concrete_to_interface(C* c)
	{
		return reinterpret_cast<I*>((char*)c - sizeof(I));
	}

	template <typename Interface, typename Concrete>
	struct bridge_t
		: Interface
	{
		bridge_t(Interface&& i, Concrete&& c)
			: Interface{std::move(i)}
			, concrete_{std::move(c)}
		{}

	protected:
		Concrete concrete_;
	};
}


namespace shiny
{
	struct resource_t : atma::ref_counted
	{
		resource_t(renderer_ptr const&, resource_type_t, resource_usage_mask_t, resource_storage_t, size_t element_stride, size_t element_count);
		virtual ~resource_t();

		auto renderer() const -> renderer_ptr const& { return rndr_; }
		auto resource_type() const -> resource_type_t { return resource_type_; }
		auto resource_usage() const -> resource_usage_mask_t { return resource_usage_; }
		auto resource_storage() const -> resource_storage_t { return resource_storage_; }
		auto elements_stride() const -> size_t { return element_stride_; }
		auto elements_count() const -> size_t { return element_count_; }
		auto resource_size() const -> size_t { return element_stride_ * element_count_; }

	private:
		renderer_ptr rndr_;
		resource_type_t resource_type_;
		resource_usage_mask_t resource_usage_;
		resource_storage_t resource_storage_;
		size_t element_stride_;
		size_t element_count_;
	};
}



#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/resource.hpp>
#endif

