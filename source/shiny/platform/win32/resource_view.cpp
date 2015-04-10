#include <shiny/platform/win32/resource_view.hpp>

#include <shiny/context.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <atma/intrusive_ptr.hpp>


using namespace shiny;
using shiny::resource_view_t;


resource_view_t::resource_view_t(resource_cptr const& rs, gpu_access_t gpua, element_format_t ef, uint offset, uint count)
	: resource_(rs), gpu_access_(gpua), format_(ef), offset_(offset), count_(count)
{
	auto fmt = platform::dxgi_format_of(format_);
	//ATMA_ASSERT(fmt != element_format_t::unknown);

	switch (gpu_access_)
	{
		case gpu_access_t::read:
		{
			auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{
				fmt,
				D3D11_SRV_DIMENSION_BUFFER,
				D3D11_BUFFER_SRV{offset_, count_}};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(resource_->d3d_resource().get(), &desc, d3d_srv_.assign()));
			break;
		}

		case gpu_access_t::read_write:
		{
			auto desc = D3D11_UNORDERED_ACCESS_VIEW_DESC{
				fmt,
				D3D11_SRV_DIMENSION_BUFFER,
				D3D11_BUFFER_UAV{offset_, count_}};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateUnorderedAccessView(resource_->d3d_resource().get(), &desc, d3d_uav_.assign()));
			break;
		}

		default:
			ATMA_HALT("bad value for gpu-access for resource-view");
			break;
	}
	
}

