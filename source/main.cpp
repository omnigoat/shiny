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

//shiny::gfx_gfx_t::create(window)



int main()
{
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise shiny
	auto shiny_runtime = shiny::scoped_runtime_t();
	auto gfx = shiny::create_context(window, shiny::primary_adapter);


	window->key_state.on_key(fooey::key_t::Ctrl + fooey::key_t::F, [&gfx]{
		std::cout << "fullscreen toggle" << std::endl;
		gfx->signal_fullscreen_toggle(1);
	});


	bool running = true;
	window->on({
		{"close", [&running](fooey::event_t&){
			running = false;
		}},

		{"resize-dc", [](fooey::events::resize_t& e) {
			//std::cout << "WM_SIZE: " << e.width() << ", " << e.height() << std::endl;
		}}
	});


	//auto vb = shiny::create_vertex_buffer({gfx}, shiny::voodoo::gpu_access_t::read,  )


	// game loop
	while (running) {
		gfx->signal_present();
		gfx->signal_block();
		//shiny::signal_present(gfx);
		//auto vb = shiny::create_vertex_buffer(gfx, )
		//auto vb = shiny::vertex_buffer_t::create(gfx, )
	}
}