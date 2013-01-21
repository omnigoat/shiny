#include <shiny/scoped_runtime.hpp>
//======================================================================
#include <shiny/plumbing/device.hpp>
#include <shiny/plumbing/prime_thread.hpp>
//======================================================================
namespace shiny {
//======================================================================

scoped_runtime_t::scoped_runtime_t() {
	plumbing::detail::setup_d3d_device();
	plumbing::prime_thread::spawn();
}

scoped_runtime_t::~scoped_runtime_t() {
	plumbing::prime_thread::join();
	plumbing::detail::teardown_d3d_device();
}

//======================================================================
} // namespace shiny
//======================================================================
        