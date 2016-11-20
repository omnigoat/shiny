#include <shiny/render_target_view.hpp>

#include <shiny/renderer.hpp>


using namespace shiny;
using shiny::render_target_view_t;


auto shiny::make_render_target_view(texture2d_ptr const& tx) -> render_target_view_ptr
{
	return render_target_view_ptr{new render_target_view_t{tx, 0}};
}




render_target_view_t::render_target_view_t(texture2d_ptr const& tx, uint mip)
	: ctx_{tx->context()}
	, texture_{tx}
	, mip_{mip}
{
	// get format, if generic, change to unsigned int
	auto fmt = tx->format();
	if (is_generic(fmt))
		fmt = (format_t)((uint32)fmt | 0x10);

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = platform::dxgi_format_of(fmt);
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D = D3D11_TEX2D_RTV{0};

	ctx_->d3d_device()->CreateRenderTargetView(texture_->d3d_resource().get(), &desc, d3d_rt_.assign());
}

