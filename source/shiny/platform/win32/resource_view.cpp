#include <shiny/platform/win32/resource_view.hpp>

#include <shiny/context.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <atma/intrusive_ptr.hpp>


using namespace shiny;
using shiny::resource_subset_t;
using shiny::resource_view_t;


resource_subset_t const resource_subset_t::whole = resource_subset_t{};


resource_view_t::resource_view_t(resource_cptr const& rs, resource_view_type_t view_type, format_t ef, resource_subset_t subset)
	: resource_(rs), resource_view_type_(view_type), format_(ef), subset_(subset)
{
	if (rs->resource_type() == resource_type_t::structured_buffer)
	{
		ATMA_ASSERT(ef == format_t::unknown, "structured buffers don't specify typed-views. use format_t::unknown");
	}

	auto fmt = platform::dxgi_format_of(format_);

	if (subset_.count == 0)
		subset_.count = rs->elements_count();

	switch (view_type)
	{
		case resource_view_type_t::input:
		{
			auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{fmt};

			switch (resource_->resource_type())
			{
				case resource_type_t::texturd3d:
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
					desc.Texture3D = D3D11_TEX3D_SRV{0, 1};
					break;


				case resource_type_t::structured_buffer:
				case resource_type_t::generic_buffer:
					desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
					desc.Buffer = D3D11_BUFFER_SRV{(UINT)subset_.offset, (UINT)subset_.count};
					break;
			}

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(resource_->d3d_resource().get(), &desc, d3d_srv_.assign()));
			d3d_view_ = d3d_srv_;
			break;
		}

		case resource_view_type_t::compute:
		{
			auto desc = D3D11_UNORDERED_ACCESS_VIEW_DESC{fmt};

			switch (resource_->resource_type())
			{
				case resource_type_t::texturd3d:
					desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
					desc.Texture3D = D3D11_TEX3D_UAV{0, 0, (UINT)-1};
					break;

				case resource_type_t::structured_buffer:
				case resource_type_t::generic_buffer:
					desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					desc.Buffer = D3D11_BUFFER_UAV{(UINT)subset_.offset, (UINT)subset_.count, 0};
					break;
			}

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateUnorderedAccessView(resource_->d3d_resource().get(), &desc, d3d_uav_.assign()));
			d3d_view_ = d3d_uav_;
			break;
		}

		case resource_view_type_t::render_target:
		{
			ATMA_ASSERT(resource_->resource_type() == resource_type_t::texture2d);
			
			auto desc = D3D11_RENDER_TARGET_VIEW_DESC{fmt};
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D = D3D11_TEX2D_RTV{0u};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateRenderTargetView(resource_->d3d_resource().get(), nullptr, &(ID3D11RenderTargetView*&)d3d_view_.get()));
			break;
		}

		case resource_view_type_t::depth_stencil:
		{
			ATMA_ASSERT(resource_->resource_type() == resource_type_t::texture2d);

			auto desc = D3D11_DEPTH_STENCIL_VIEW_DESC{fmt};
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D = D3D11_TEX2D_DSV{0u};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateDepthStencilView(resource_->d3d_resource().get(), &desc, &(ID3D11DepthStencilView*&)d3d_view_.get()));
			break;
		}

		default:
			ATMA_HALT("bad!!");
			break;
	}
}

resource_view_t::~resource_view_t()
{
}

auto resource_view_t::context() const -> renderer_ptr const&
{
	return resource_->context();
}

auto shiny::make_resource_view(resource_cptr const& r, resource_view_type_t vt, format_t ef, resource_subset_t s) -> resource_view_ptr
{
	return atma::make_intrusive<resource_view_t>(r, vt, ef, s);
}


shiny::bound_resource_view_t::bound_resource_view_t(uint idx, resource_view_cptr const& view)
	: idx(idx), view(view), counter(~uint())
{
}

shiny::bound_resource_view_t::bound_resource_view_t(uint idx, resource_view_cptr const& view, uint counter)
	: idx(idx), view(view), counter(counter)
{
}