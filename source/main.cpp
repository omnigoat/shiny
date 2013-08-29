#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <atma/evented/event.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

int main()
{
#if 0
	atma::math::vector4f v{1, 2, 3, 4};
	auto addition = v + v;
	//atma::math::vector4f r = addition;
	auto multiplication = addition * 3.f;
	atma::math::vector4f r2 = multiplication;

	atma::math::matrix4f m = atma::math::matrix4f::identity();
	atma::math::matrix4f m2 = atma::math::matrix4f::identity();

	m.set(0, 0, 4.f);
	m.set(1, 1, 2.f);
	m.set(2, 2, 4.f / 3.f);

	auto rv = m * v;

	m2.set(1, 1, 3.f);

	

	auto mr = m * m2;
	auto k = atma::math::dot_product(v, v);

	return (int)v[2];
#else
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto wnd = fooey::window("Excitement.");
	renderer->add_window(wnd);

	// runtime!
	auto SR = shiny::scoped_runtime_t();
	// context per window!
	//auto context = shiny::rendering_context_t(wnd);
	
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
	});


	
	
	bool running = true;

	wnd->on_minimise += [](atma::evented::flowcontrol_t& fc) {
		std::cout << "bam, minimised" << std::endl;
		fc.stop_execution();
		fc.prevent_default_behaviour();
	};
	
	wnd->on_minimise += [](atma::evented::flowcontrol_t&) {
		std::cout << "you jelly, Qt?" << std::endl;
	};

	wnd->on_maximise += [](atma::evented::flowcontrol_t&) {
		std::cout << "wow, maximised" << std::endl;
	};

	wnd->on_resize += [](atma::evented::flowcontrol_t&) {
		std::cout << "WM_SIZE" << std::endl;
	};

#if 0
	wnd->on_resize += [](uint32_t width, uint32_t height) {
		shiny::signal_resize(width, height);
	};
#endif

	wnd->on_restore += [](atma::evented::flowcontrol_t&) {
		std::cout << "ooh, restored" << std::endl;
	};

	wnd->on_close += [&running](atma::evented::flowcontrol_t&) {
		std::cout << "lol, bye" << std::endl;
		running = false;
	};

	// game loop
	while (running)
		fooey::process_events(wnd);
#endif
}