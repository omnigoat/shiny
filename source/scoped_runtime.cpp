#include <shiny/scoped_runtime.hpp>
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/thread.hpp>
//======================================================================
#include <algorithm>

using shiny::scoped_runtime_t;

scoped_runtime_t::scoped_runtime_t() {
	voodoo::prime_thread::spawn();
}

scoped_runtime_t::~scoped_runtime_t() {

	std::for_each(threads_.begin(), threads_.end(), std::mem_fn(&std::thread::join));
	
	voodoo::prime_thread::join();
}

auto scoped_runtime_t::add_thread(std::function<void()> fn) -> void
{
	threads_.push_back(std::thread(fn));
}

auto scoped_runtime_t::add_context_thread(std::function<void()> fn) -> void
{
	threads_.push_back(voodoo::spawn_context_thread(fn));
}
