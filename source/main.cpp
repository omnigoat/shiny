#include <dust/scoped_runtime.hpp>
//#include <dust/vertex_buffer.hpp>
#include <iostream>


#include <atma/intrusive_ptr.hpp>
#include <dust/thread.hpp>
#include <dust/command.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/context.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

int main()
{
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise dust
	auto dust_runtime = dust::scoped_runtime_t();
	auto gfx = dust::create_context(window, dust::primary_adapter);


	window->key_state.on_key(fooey::key_t::Ctrl + fooey::key_t::F, [&gfx, window]{
		gfx->signal_fullscreen_toggle(1);
	});


	auto vb = dust::plumbing::vertex_buffer_t::create(gfx, dust::plumbing::vertex_buffer_t::usage_t::long_lived, true, 3, nullptr);
	dust::plumbing::locked_vertex_buffer_t lvb(*vb, dust::plumbing::lock_type_t::write);
	


	bool running = true;
	window->on({
		{"close", [&running](fooey::event_t&){
			running = false;
		}},

		{"resize-dc", [](fooey::events::resize_t& e) {
			//std::cout << "WM_SIZE: " << e.width() << ", " << e.height() << std::endl;
		}}
	});


	//auto vb = dust::create_vertex_buffer({gfx}, dust::voodoo::gpu_access_t::read,  )


	// game loop
	while (running) {
		gfx->signal_present();
		gfx->signal_block();
		//dust::signal_present(gfx);
		//auto vb = dust::create_vertex_buffer(gfx, )
		//auto vb = dust::vertex_buffer_t::create(gfx, )
	}
}
