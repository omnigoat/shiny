#include <shiny/scene.hpp>

#include <shiny/context.hpp>
#include <shiny/camera.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/data_declaration.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>
#include <shiny/render_target_view.hpp>
#include <shiny/draw_target.hpp>

#include <atma/shared_memory.hpp>


using namespace shiny;
using shiny::scene_t;


namespace
{
	struct scene_data_t
	{
		atma::math::matrix4f view;
		atma::math::matrix4f inv_view;
		atma::math::matrix4f proj;
		atma::math::matrix4f inv_viewproj;
		float time;
	};
}


scene_t::scene_t(context_ptr const& context, draw_target_ptr const& dt, camera_t const& camera, rendertarget_clear_t const& fc)
	: context_(context)
	, draw_target_(dt)
	, camera_(&camera)
{
	if (fc.clear_any())
	{
		batch_.push([&] {
			context_->immediate_set_render_target(0, dt->render_target());
			context_->immediate_set_depth_stencil(0, dt->depth_stencil_target());
			context_->immediate_clear(fc);
		});
	}

	scene_constant_buffer_ = shiny::make_constant_buffer(context_, scene_data_t{
		camera.view(),
		camera.inverse_view(),
		camera.projection(),
		invert(camera.view() * camera.projection()),
		0.f
	});
}

scene_t::scene_t(context_ptr const& context, camera_t const& camera, rendertarget_clear_t const& fc)
	: context_(context), camera_(&camera)
{
	if (fc.clear_any())
	{
		batch_.push([&] {
			context_->immediate_clear(fc);
		});
	}

	scene_constant_buffer_ = shiny::make_constant_buffer(context_, scene_data_t{
		camera.view(),
		camera.inverse_view(),
		camera.projection(),
		invert(camera.view() * camera.projection()),
		0.f
	});
}


