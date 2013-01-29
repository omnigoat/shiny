#include <shiny/scoped_runtime.hpp>
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/plumbing/prime_thread.hpp>
//======================================================================
namespace shiny {
//======================================================================

scoped_runtime_t::scoped_runtime_t() {
	voodoo::setup_d3d_device();
	plumbing::prime_thread::spawn();
}

scoped_runtime_t::~scoped_runtime_t() {
	plumbing::prime_thread::join();
	voodoo::teardown_d3d_device();
}

//======================================================================
} // namespace shiny
//======================================================================
        