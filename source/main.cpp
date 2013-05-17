#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>

#include <atma/intrusive_ptr.hpp>
#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/command.hpp>

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
	shiny::scoped_runtime_t SR;
	auto k = []{std::cout << "hello" << std::endl;};
	#if 0
	shiny::voodoo::command_ptr C = shiny::voodoo::make_command(&hello);
	C->operator ()();
	shiny::voodoo::command_ptr C2 = shiny::voodoo::make_command(&zomg::operator(), zomg());
	C2->operator ()();
	#endif
	//shiny::plumbing::vertex_buffer_t vb(shiny::plumbing::vertex_buffer_t::usage::general, 48, false);

	//auto k = vb.lock<char>(shiny::plumbing::lock_type_t::write);
	/*shiny::voodoo::spawn_thread([] {
		std::cout << "bam!" << std::endl;
	});*/
}