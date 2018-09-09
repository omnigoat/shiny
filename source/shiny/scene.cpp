#include <shiny/scene.hpp>

#include <shiny/renderer.hpp>
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

scene_t::scene_t(scene_t&& rhs)
	: rndr_{std::move(rhs.rndr_)}
	, draw_target_{std::move(rhs.draw_target_)}
	, clear_{std::move(rhs.clear_)}
	, camera_{std::move(rhs.camera_)}
	, scene_constant_buffer_{std::move(rhs.scene_constant_buffer_)}
	, draw_commands_{std::move(rhs.draw_commands_)}
{}

scene_t::scene_t(renderer_ptr const& renderer, draw_target_t const& dt, camera_t const& camera, rendertarget_clear_t const& fc)
	: rndr_(renderer)
	, draw_target_(dt)
	, clear_{fc}
	, camera_(camera)
{
	scene_constant_buffer_ = rndr_->make_constant_buffer_for(
		scene_data_t{
			camera.view(),
			camera.inverse_view(),
			camera.projection(),
			invert(camera.view() * camera.projection()),
			0.f});
}

scene_t::scene_t(renderer_ptr const& renderer, draw_target_t const& dt)
	: scene_t(renderer, dt, camera_t{}, rendertarget_clear_t{})
{}

scene_t::scene_t(renderer_ptr const& renderer, camera_t const& camera, rendertarget_clear_t const& fc)
	: rndr_(renderer)
	, camera_(camera)
	, clear_{fc}
{
	scene_constant_buffer_ = rndr_->make_constant_buffer_for(
		scene_data_t{
			camera.view(),
			camera.inverse_view(),
			camera.projection(),
			invert(camera.view() * camera.projection()),
			0.f});
}


