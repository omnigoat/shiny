#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>


#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>
#include <shiny/context.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

//shiny::gfx_context_t::create(window)

int main()
{
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise shiny
	auto SR = shiny::scoped_runtime_t();
	auto context = shiny::create_context(window, shiny::primary_adapter);

	bool running = true;

	window->key_state.on_key(fooey::key_t::Alt + fooey::key_t::Enter, [&context]{
		shiny::signal_fullscreen_toggle(context);
	});

	window->on({
		{"close", [&running](fooey::event_t&){
			running = false;
		}},

		{"resize-dc", [](fooey::events::resize_t& e) {
			//std::cout << "WM_SIZE: " << e.width() << ", " << e.height() << std::endl;
		}}
	});

	
	
	//shiny::spawn_gfx_thread(context, [] {});

	// game loop
	while (running) {
		//shiny::signal_present(context);
		//auto vb = shiny::create_vertex_buffer(context, )
		//auto vb = shiny::vertex_buffer_t::create(context, )
	}
}