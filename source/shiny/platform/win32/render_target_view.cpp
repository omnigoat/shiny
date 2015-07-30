#include <shiny/platform/win32/render_target_view.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using shiny::render_target_view_t;

render_target_view_t::render_target_view_t(texture2d_ptr const& tx, uint mip)
	: ctx_{tx->context()}
	, texture_{tx}
	, mip_{mip}
{
	auto desc = D3D11_RENDER_TARGET_VIEW_DESC{};

	ctx_->d3d_device()->CreateRenderTargetView(texture_->d3d_resource().get(), &desc, d3d_rt_.assign());
}
