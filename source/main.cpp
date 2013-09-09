#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>

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
	auto wnd = fooey::window("Excitement.", 480, 360);
	renderer->add_window(wnd);

	// runtime!
	auto SR = shiny::scoped_runtime_t();
	// context per window!
	//auto context = shiny::rendering_context_t(wnd);
	
	
	
	
	bool running = true;

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

	wnd->on_resize += [](atma::event_flow_t&, uint32_t width, uint32_t height) {
		std::cout << "WM_SIZE: " << width << ", " << height << std::endl;
	};

#if 0
	wnd->on_resize += [](uint32_t width, uint32_t height) {
		shiny::signal_resize(width, height);
	};
#endif

	wnd->on_restore += [](atma::event_flow_t&) {
		std::cout << "ooh, restored" << std::endl;
	};

	wnd->on_close += [&running](atma::event_flow_t&) {
		std::cout << "lol, bye" << std::endl;
		running = false;
	};

	//{fooey::keys::Alt + fooey::key_t::Enter}
	//{fooey::keys::Ctrl + fooey::keys::K, fooey::keys::Ctrl + fooey::Keys::C};



	auto context = shiny::voodoo::create_context(wnd, 0, 0);

	wnd->key_state.on_key(fooey::key_t::Ctrl + fooey::key_t::F, [&context]{
		//context->toggle_fullscreen();
		std::cout << "blam" << std::endl;
	});
/*
	auto keymappings =
		shiny::map_keys_for(wnd)
			.add_key(fooey::keys::A, )
			;
	//shiny::unmap_keys(wnd, keymappings);

	shiny::on_key_chord({shiny::keys::Ctrl + shiny::keys::Backtick}, []{

	});

*/

	// game loop
	while (running)
		;
#endif
}