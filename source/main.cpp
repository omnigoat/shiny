#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>

#include <fooey/fooey.hpp>

int main()
{
	shiny::scoped_runtime_t SR;
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
	});

	fooey::window_t W("blam");

}