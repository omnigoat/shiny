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

int main()
{

	atma::math::vector4f v{1, 2, 3, 4};
	auto addition = v + v;
	//atma::math::vector4f r = addition;
	auto multiplication = addition * 3.f;
	atma::math::vector4f r2 = multiplication;

	
	
	auto k = atma::math::dot_product(v, v);

	return (int)v[2];
#if 0
	auto SR = shiny::scoped_runtime_t{};
	
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
	});


	// setup up gui
	auto renderer = fooey::system_renderer();
	
	auto wnd = fooey::window("Excitement.");
	renderer->add_window(wnd);
	
	bool running = true;

	wnd->on_minimise += [] {
		std::cout << "bam, minimised" << std::endl;
	};

	/*wnd->on_minimise += [] {
		std::cout << "you jelly, Qt?" << std::endl;
	};*/

	wnd->on_maximise += [] {
		std::cout << "wow, maximised" << std::endl;
	};

	wnd->on_restore += [] {
		std::cout << "ooh, restored" << std::endl;
	};

	wnd->on_close += [&running] {
		std::cout << "lol, bai" << std::endl;
		running = false;
	};

	// game loop
	while (running)
		fooey::process_events(wnd);
#endif
}