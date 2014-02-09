#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

#include <iostream>

int main()
{
	bool running = true;


	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	// initialise dust
	auto dust_runtime = dust::runtime_t();
	auto gfx = dust::create_context(dust_runtime, window, dust::primary_adapter);

	// create vb
	float D[] = {
		 0.f,    0.5f, 0.f, 1.f,
		 0.45f, -0.5f, 0.f, 1.f,
		-0.45f, -0.5f, 0.f, 1.f
	};

	auto vs = dust::vertex_shader_ptr(new dust::vertex_shader_t(gfx));
	auto ps = dust::pixel_shader_ptr(new dust::pixel_shader_t(gfx));

	auto vb = dust::create_vertex_buffer(gfx, dust::vb_usage_t::immutable, 16 * sizeof(float), D);
	auto vd = dust::vertex_declaration_t(gfx, vs, {
		{dust::vertex_stream_t::usage_t::position, 0, dust::vertex_stream_t::element_type_t::float32, 4}
	});

	
	window->key_state.on_key(fooey::key_t::Alt + fooey::key_t::Enter, [gfx]{
		gfx->signal_fullscreen_toggle();
	});


	while (running) {

		gfx->signal_block();
		gfx->signal_clear();
		gfx->signal_draw(vd, vb, vs, ps);
		gfx->signal_present();
	}
}
