#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/constant_buffer.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

#include <iostream>

int main()
{
	
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise dust
	auto dust_runtime = dust::runtime_t();
	auto gfx = dust::create_context(dust_runtime, window, dust::primary_adapter);



	// shaders
	auto vs = dust::create_vertex_shader(gfx);
	auto ps = dust::create_pixel_shader(gfx);

	// vertex declaration
	auto vd = dust::vertex_declaration_t(gfx, vs, {
		{dust::vertex_stream_t::usage_t::position, 0, dust::vertex_stream_t::element_type_t::float32, 4}
	});
	
	// create vb
	float D[] = {
		 0.f,  1.f, 0.f, 1.f,
		 1.f, -1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 1.f
	};

	

	
	auto vb = dust::create_vertex_buffer(gfx, dust::vb_usage_t::immutable, vd, 3, D);

	namespace math = atma::math;
	
	auto view = math::look_at(math::point4f(2.f, 4.f, 2.f), math::point4f(0.f, 0.f, 0.f), math::vector4f(0.f, 1.f, 0.f, 0.f));
	auto proj = math::pespective(800, 600, 0.1f, 100.f);

	auto m1 = math::matrix4f::identity();
	auto m2 = math::matrix4f::identity();
	m1[0][3] = 4.f;
	m2[0][0] = 2.f;
	auto v1 = math::point4f();
	auto r = v1 * m1 * m2;

	
	//auto scene = dust::scene_queue_t(view, proj);
	// gfx->signal_constant_buffer_upload(0, my_constant_buffer)

	bool running = true;

	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	window->key_state.on_key(fooey::key_t::Alt + fooey::key_t::Enter, [gfx]{
		gfx->signal_fullscreen_toggle(1);
	});


	while (running) {
		auto cb = dust::create_constant_buffer(gfx);

		gfx->signal_block();
		gfx->signal_upload_constant_buffer(0, cb);
		gfx->signal_clear();
		gfx->signal_draw(vd, vb, vs, ps);
		gfx->signal_present();
	}
}
