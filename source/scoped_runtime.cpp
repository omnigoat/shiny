#include <shiny/scoped_runtime.hpp>
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/thread.hpp>
//======================================================================
namespace shiny {
//======================================================================

scoped_runtime_t::scoped_runtime_t() {
	voodoo::setup_d3d_device();
	voodoo::prime_thread::spawn();
}

scoped_runtime_t::~scoped_runtime_t() {
	voodoo::prime_thread::join();
	voodoo::teardown_d3d_device();
}

//======================================================================
} // namespace shiny
//======================================================================

