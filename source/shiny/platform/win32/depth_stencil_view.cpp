#include <shiny/platform/win32/depth_stencil_view.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using shiny::depth_stencil_view_t;

depth_stencil_view_t::depth_stencil_view_t(texture2d_ptr const& tx, uint mip)
	: ctx_{tx->context()}
	, texture_{tx}
	, mip_{mip}
{
	// convert format into bestest depth-format
	auto fmt = tx->format();
	ATMA_ASSERT(format_depth_size(fmt) > 0);

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Format = platform::dxgi_format_of(fmt);
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D = D3D11_TEX2D_DSV{0};

	ctx_->d3d_device()->CreateDepthStencilView(texture_->d3d_resource().get(), &desc, d3d_rt_.assign());
}