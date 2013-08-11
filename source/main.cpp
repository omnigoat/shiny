#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>


int main()
{
	shiny::scoped_runtime_t SR;
	
	SR.add_context_thread([] {
		float f[] = { 1, 2, 3, 4 };
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
		shiny::plumbing::locked_vertex_buffer_t lvb(vb, shiny::plumbing::lock_type_t::write_discard);
		auto i = lvb.begin<float>();
		std::copy(f, f + 4, i);
	});
}