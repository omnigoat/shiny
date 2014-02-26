#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/index_buffer.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

#include <iostream>

#include <DirectXMath.h>

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
		{dust::vertex_stream_t::usage_t::position, 0, dust::vertex_stream_t::element_type_t::float32, 4},
		{dust::vertex_stream_t::usage_t::color, 0, dust::vertex_stream_t::element_type_t::float32, 4}
	});
	
	// create vb
	float D[] = {
		 0.5f,  0.5f,  0.5f, 1.f,   1.f, 0.f, 0.f, 1.f,
		 0.5f,  0.5f, -0.5f, 1.f,   0.f, 1.f, 0.f, 1.f,
		 0.5f, -0.5f,  0.5f, 1.f,   0.f, 0.f, 1.f, 1.f,
		 0.5f, -0.5f, -0.5f, 1.f,   1.f, 1.f, 0.f, 1.f,
		-0.5f,  0.5f,  0.5f, 1.f,   1.f, 0.f, 1.f, 1.f,
		-0.5f,  0.5f, -0.5f, 1.f,   0.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f,  0.5f, 1.f,   1.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f, -0.5f, 1.f,   1.f, 0.f, 0.f, 1.f
	};

	uint16 IBD[] = {
		4, 5, 7, 7, 6, 4, // -x plane
		0, 2, 3, 3, 1, 0, // +x plane
		5, 1, 3, 3, 7, 5, // -z plane
		6, 2, 0, 0, 4, 6, // +z plane
		0, 1, 5, 5, 4, 0, // +y plane
		2, 6, 7, 7, 3, 2  // -y plane
	};


	
	auto vb = dust::create_vertex_buffer(gfx, dust::buffer_usage_t::immutable, vd, 8, D);

	auto ib = dust::create_index_buffer(gfx, dust::buffer_usage_t::immutable, 16, 36, IBD);

	namespace math = atma::math;
	
	
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

	struct B
	{
		atma::math::matrix4f world;
		atma::math::matrix4f view;
		atma::math::matrix4f proj;
		float time;
	};

	B b;
	static float t = 1.f;

	auto cb = dust::create_constant_buffer(gfx, sizeof(B), &b);



	while (running)
	{

		b.time = t;
		t += 0.1f;

		namespace math = atma::math;
		static float x = 0.f;
		static float y = 0.f;
		if (GetAsyncKeyState(VK_LEFT))
			x -= 0.001f;
		else if (GetAsyncKeyState(VK_RIGHT))
			x += 0.001f;
		else if (GetAsyncKeyState(VK_UP))
			y += 0.001f;
		else if (GetAsyncKeyState(VK_DOWN))
			y -= 0.001f;

		using namespace DirectX;
		auto V = XMMatrixLookAtLH(XMVectorSet(sin(x) * cos(y) * 2.f, sin(y) * 2.f, cos(x) * cos(y) * 2.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 0.f), XMVectorSet(0.f, 1.f, 0.f, 0.f));
		auto P = XMMatrixPerspectiveFovLH(XM_PIDIV2, 480.f / 360.f, 0.01f, 100.f);


		b.world = math::rotation_y(t * 0.002f);
		b.view = math::look_at(math::point4f(sin(x) * cos(y) * 2.f, sin(y) * 2.f, cos(x) * cos(y) * 2.f), math::point4f(0.f, 0.f, 0.f), math::vector4f(0.f, 1.f, 0.f, 0.f));
		b.proj = math::perspective_fov(math::pi_over_two, 480.f / 360.f, 0.01f, 100.f);


		cb->signal_upload_new_data(&b);

		gfx->signal_block();
		gfx->signal_clear();
		gfx->signal_upload_constant_buffer(0, cb);
		//gfx->signal_draw(vd, vb, vs, ps);
		gfx->signal_draw(ib, vd, vb, vs, ps);
		gfx->signal_present();
	}
}
