#include <shiny/plumbing/device.hpp>

int blah()
{
	shiny::plumbing::context_t c2;
	return 4;
}

int main()
{
	shiny::plumbing::context_t c;

	std::thread g(blah);
	g.join();
}