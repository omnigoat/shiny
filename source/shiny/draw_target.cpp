#include <shiny/draw_target.hpp>

#include <shiny/render_target_view.hpp>
#include <shiny/depth_stencil_view.hpp>

using namespace shiny;
using shiny::draw_target_t;


draw_target_t::draw_target_t()
{}

draw_target_t::draw_target_t(resource_view_ptr const& rtv, resource_view_ptr const& dsv)
	: render_target_{rtv}
	, depth_stencil_{dsv}
{}

draw_target_t::draw_target_t(resource_view_ptr const& rtv)
	: render_target_{rtv}
{}