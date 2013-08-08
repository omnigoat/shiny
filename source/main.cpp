#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>

void hello()
{
	std::cout << "hello" << std::endl;
}

struct zomg {
	void operator ()() {
		std::cout << "zomg" << std::endl;
	}
};

int main()
{
	//shiny::scoped_runtime_t SR;
	shiny::voodoo::prime_thread::spawn();

	auto t = shiny::voodoo::spawn_thread([] {
		// vertex buffer of 16 bytes for a quad
		float f[] = {1, 2, 3, 4};
		shiny::plumbing::vertex_buffer_t vb(shiny::voodoo::gpu_access_t::read, shiny::voodoo::cpu_access_t::write, true, 16, f);
		shiny::plumbing::locked_vertex_buffer_t lvb(vb, shiny::plumbing::lock_type_t::write_discard);
	});

	t.join();

	

	//auto k = []{std::cout << "hello" << std::endl;};
	//shiny::voodoo::command_ptr C = shiny::voodoo::make_command(&hello);

	#if 0

	C->operator ()();
	shiny::voodoo::command_ptr C2 = shiny::voodoo::make_command(&zomg::operator(), zomg());
	C2->operator ()();
	#endif
	
	shiny::voodoo::prime_thread::join();
}