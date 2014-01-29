#include <dust/runtime.hpp>
#include <dust/context.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>

#include <iostream>


int main()
{
#if 1
	// setup up gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);
#endif

	// initialise dust
	auto dust_runtime = dust::runtime_t();
	auto gfx = dust::create_context(dust_runtime, window, dust::primary_adapter);
}
