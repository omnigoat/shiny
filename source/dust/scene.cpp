#include <dust/scene.hpp>

#include <dust/context.hpp>
#include <dust/camera.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/index_buffer.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/fragment_shader.hpp>

#include <atma/shared_memory.hpp>


using namespace dust;
using dust::scene_t;


scene_t::scene_t(context_ptr const& context, camera_t const& camera)
	: context_(context)
{
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
		(camera.projection() * camera.view()).inverted(),
		0.f
	};

	scene_buffer_ = dust::create_constant_buffer(context_, sd);

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

auto scene_t::signal_draw(index_buffer_ptr const& ib, vertex_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, pixel_shader_ptr const& ps) -> void
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


