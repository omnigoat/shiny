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

void print_4(int a, int b, int c, int d) {
	//std::cout << "blah: " << a << ", " << b << ", " << c << ", " << d << std::endl;
}

auto add(std::shared_ptr<int> const& a, int b) -> int {
	return *a + b;
}

struct printer_t
{
	printer_t() : x(-1) {}
	printer_t(int x) : x(x) {}

	void print_4_x(int a, int b, int c, int d)
	{
		std::cout << "blah: " << a << ", " << b << ", " << c << ", " << d << ", " << x << std::endl;
	}

	int x;
};

static atma::xtm::detail::xtm_ph<0> _1;
static atma::xtm::detail::xtm_ph<1> _2;
static atma::xtm::detail::xtm_ph<2> _3;


int main()
{
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
}