#include <shiny/draw_target.hpp>

#include <shiny/render_target_view.hpp>
#include <shiny/depth_stencil_view.hpp>


using namespace shiny;
using shiny::draw_target_t;

draw_target_t::draw_target_t(render_target_view_ptr const& rtv, depth_stencil_view_ptr const& dsv)
	: render_target_{rtv}
	, depth_stencil_target_{dsv}
{}

//draw_target_t(render_target_ptr const&);