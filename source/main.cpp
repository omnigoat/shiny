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
	std::cout << "blah: " << a << ", " << b << ", " << c << ", " << d << std::endl;
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
	// function pointer
	auto b = atma::xtm::bind(&print_4, 1, 2, _2, _1);
	b(4, 3);
	
	// member function pointer
	printer_t printer(5);
	auto b2 = atma::xtm::bind(&printer_t::print_4_x, _3, 1, _2, 3, _1);
	b2(4, 2, &printer);

	// lambda
	auto L = [](int a, int b) -> int {return a + b; };
	auto b3 = atma::xtm::bind(L, 7, _1);
	std::cout << b3(3) << std::endl;
	
	
	

	return 0;

	auto SR = shiny::scoped_runtime_t{};
	
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
	});


	// setup up gui
	auto wnd = fooey::window("blam");
	auto R = fooey::system_renderer();
	R->register_window(wnd);

	
	// game loop
#if 0
	while (;;)
		fooey::process_events(wnd);
#endif
	




	#if 0
	
	
	//fooey::map_close_to_shutdown(wnd);

	wnd->window_events.on_minimise += [](fooey::window_ptr const& x, et const&) {
		x->set_window_state(fooey::)
	};
	
	fooey::map_window_events(wnd,
	{
		{fooey::window_event_t::minimise, [](wp const& x, et const&) {
			fooey::set_window_state(x, fooey::window_t::minimised);
		}},
		{fooey::window_event_t::maximise, [](wp const& x, et const&) {
			fooey::set_window_state(x, fooey::window_t::maximised);
		}}
	});
	#endif
	for (;;)
		;
}