#include <shiny/platform/win32/resource_view.hpp>

#include <shiny/context.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <atma/intrusive_ptr.hpp>


using namespace shiny;
using shiny::resource_subset_t;
using shiny::resource_view_t;


resource_subset_t const resource_subset_t::whole = resource_subset_t{};


resource_view_t::resource_view_t(resource_cptr const& rs, view_type_t view_type, gpu_access_t gpua, element_format_t ef, resource_subset_t subset)
	: resource_(rs), view_type_(view_type), gpu_access_(gpua), format_(ef), subset_(subset)
{
	if (rs->usage_flags() & resource_usage_t::structured)
	{
		ATMA_ASSERT_MSG(ef == element_format_t::unknown, "structured buffers don't specify typed-views. use element_format_t::unknown");
	}

	auto fmt = platform::dxgi_format_of(format_);
	
	//if (subset_.count == 0)
		//subset_.count = resource_->

	switch (gpu_access_)
	{
		case gpu_access_t::read:
		{
			auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{
				fmt,
				D3D11_SRV_DIMENSION_BUFFER,
				D3D11_BUFFER_SRV{subset_.offset, 4}};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(resource_->d3d_resource().get(), &desc, d3d_srv_.assign()));
			break;
		}

		case gpu_access_t::read_write:
		{
			auto desc = D3D11_UNORDERED_ACCESS_VIEW_DESC{
				fmt,
				D3D11_UAV_DIMENSION_BUFFER,
				D3D11_BUFFER_UAV{subset_.offset, subset_.count}};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateUnorderedAccessView(resource_->d3d_resource().get(), &desc, d3d_uav_.assign()));
			break;
		}

		default:
			ATMA_HALT("bad value for gpu-access for resource-view");
			break;
	}
	
}

auto resource_view_t::context() const -> context_ptr const&
{
	return resource_->context();
}

auto shiny::make_resource_view(resource_cptr const& r, gpu_access_t ga, element_format_t ef, resource_subset_t s) -> resource_view_ptr
{
	return atma::make_intrusive_ptr<resource_view_t>(r, view_type_t::buffer, ga, ef, s);
}


