#include <shiny/scene.hpp>

#include <shiny/context.hpp>
#include <shiny/camera.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/data_declaration.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>

#include <atma/shared_memory.hpp>


using namespace shiny;
using shiny::scene_t;


scene_t::scene_t(context_ptr const& context, camera_t const& camera, rendertarget_clear_t const& fc)
	: context_(context)
{
	if (fc.clear())
	{
		queue_.push([&] {
			context_->signal_clear(fc.color());
		});
	}

	struct scene_data_t
	{
		atma::math::matrix4f view;
		atma::math::matrix4f proj;
		atma::math::matrix4f inv_viewproj;
		float time;
	};

	auto sd = scene_data_t{
		camera.view(),
		camera.projection(),
		invert(camera.projection() * camera.view()),
		0.f
	};

	scene_buffer_ = shiny::create_constant_buffer(context_, sd);

	queue_.push([&] {
		context_->signal_cs_upload_constant_buffer(0, scene_buffer_);
	});
}

auto scene_t::execute() -> void
{
	atma::thread::engine_t::signal_t s;
	while (queue_.pop(s))
		s();
}

auto scene_t::signal_cs_upload_constant_buffer(uint index, constant_buffer_cptr const& buf) -> void
{
	ATMA_ASSERT_MSG(index > 0, "constant-buffer[0] is reserved for the scene");

	queue_.push([&, index, buf] {
		context_->signal_cs_upload_constant_buffer(index, buf);
	});
}

auto scene_t::signal_draw(index_buffer_ptr const& ib, data_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, fragment_shader_ptr const& ps) -> void
{
	queue_.push([&, ib, vd, vb, vs, ps] {
		context_->signal_draw(ib, vd, vb, vs, ps);
	});
}

auto scene_t::signal_res_update(constant_buffer_ptr& cb, uint data_size, void* data) -> void
{
	auto sm = atma::shared_memory_t(data_size, data);

	queue_.push([&, cb, sm] {
		context_->signal_res_update(cb, sm);
	});
}

