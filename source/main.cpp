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

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

struct dragon_event_t : fooey::event_t
{
};

int main()
{
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto wnd = fooey::window("Excitement.", 480, 360);
	renderer->add_window(wnd);

	// runtime!
	auto SR = shiny::scoped_runtime_t();
	// context per window!
	auto context = shiny::create_context(shiny::defer_construction);
	//context->bind_to(wnd);
	
	bool running = true;

	wnd->on("close.dragons", [](dragon_event_t& e){
		std::cout << "close.dragon" << std::endl;
	});

	wnd->on("close.dragons.things", [](dragon_event_t& e){
		std::cout << "close.dragon.things" << std::endl;
	});

	wnd->fire("close.things", dragon_event_t());

#if 0
	wnd->on_minimise += [](atma::event_flow_t& fc) {
		std::cout << "bam, minimised" << std::endl;
		fc.stop_execution();
		fc.prevent_default_behaviour();
	};
	
	wnd->on_minimise += [](atma::event_flow_t&) {
		std::cout << "you jelly, Qt?" << std::endl;
	};

	wnd->on_maximise += [](atma::event_flow_t&) {
		std::cout << "wow, maximised" << std::endl;
	};

	

	wnd->on_resize += [](uint32_t width, uint32_t height) {
		shiny::signal_resize(width, height);
	};


	wnd->on_restore += [](atma::event_flow_t&) {
		std::cout << "ooh, restored" << std::endl;
	};

#endif
	wnd->on_resize.add("shiny", [](fooey::widget_event_t const&, uint32_t width, uint32_t height) {
		std::cout << "WM_SIZE: " << width << ", " << height << std::endl;
	});

	wnd->on_close.add("shiny", [&running](fooey::widget_event_t const&) {
		std::cout << "lol, bye" << std::endl;
		running = false;
	});

	#if 0
	wnd->key_state.on_key(fooey::key_t::Ctrl + fooey::key_t::F, [&context]{
		shiny::signal_fullscreen_toggle(context);
	});
	#endif


	// game loop
	while (running)
		;
}