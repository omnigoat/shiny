#include <shiny/scoped_runtime.hpp>
//======================================================================
#include <shiny/plumbing/device.hpp>
#include <shiny/plumbing/prime_thread.hpp>
//======================================================================
namespace shiny {
//======================================================================

scoped_runtime_t::scoped_runtime_t() {
	plumbing::detail::setup_d3d_device();
	plumbing::detail::spawn_prime_thread();
}

scoped_runtime_t::~scoped_runtime_t() {
	plumbing::detail::join_prime_thread();
	plumbing::detail::teardown_d3d_device();
}

//======================================================================
} // namespace shiny
//======================================================================
        