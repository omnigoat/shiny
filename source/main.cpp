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

void blah() {
	std::cout << "blah" << std::endl;
}

int main()
{
	shiny::scoped_runtime_t SR;
	
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
	});


	// setup up gui
	fooey::window_ptr wnd = fooey::window("blam");
	
	fooey::renderer_ptr R = fooey::system_renderer();
	R->register_window(wnd);

	//fooey::input::register_ui(wnd);

	wnd->on_minimise += blah;
	wnd->on_minimise.fire();
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