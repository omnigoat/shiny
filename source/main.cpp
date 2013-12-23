#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>


#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>
#include <shiny/voodoo/context.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

int main()
{
	// setup up gui
	auto renderer = fooey::system_renderer();
	fooey::window_ptr window = fooey::window("Excitement.", 480, 360);


	renderer->add_window(window);

	// runtime!
	auto SR = shiny::scoped_runtime_t();
	// context per window!
	//auto context = shiny::create_context(window);
	
	bool running = true;

	//window->key_state.on_key(fooey::key_t::Alt + fooey::key_t::Enter, [&context]{
	//	context->toggle_fullscreen();
	//});

	window->on({
		{"close", [&running](fooey::event_t&){
			running = false;
		}},

		{"resize-dc", [](fooey::events::resize_t& e) {
			std::cout << "WM_SIZE: " << e.width() << ", " << e.height() << std::endl;
		}}
	});


	// game loop
	while (running)
		;
}