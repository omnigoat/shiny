#include <shiny/scoped_runtime.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>

int main()
{
	shiny::scoped_runtime_t SR;
	
	shiny::plumbing::vertex_buffer_t vb(shiny::plumbing::vertex_buffer_t::usage::general, 48, false);

	auto k = vb.lock<char>(shiny::plumbing::lock_type_t::write);

}