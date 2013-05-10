//#include <shiny/scoped_runtime.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>
#include <iostream>
//#include <shiny/voodoo/thread.hpp>
#include <atma/intrusive_ptr.hpp>
int main()
{
//	shiny::scoped_runtime_t SR;
	struct S : atma::ref_counted {S():s(1){} int s;};
	struct T : S {T() : t(2) {} int t;};

	atma::intrusive_ptr<S> s;
	atma::intrusive_ptr<T> t(s);
	t = new S;
	//shiny::plumbing::vertex_buffer_t vb(shiny::plumbing::vertex_buffer_t::usage::general, 48, false);

	//auto k = vb.lock<char>(shiny::plumbing::lock_type_t::write);
	/*shiny::voodoo::spawn_thread([] {
		std::cout << "bam!" << std::endl;
	});*/
}