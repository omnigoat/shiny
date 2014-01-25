#include <dust/scoped_runtime.hpp>
//======================================================================
#include <dust/device.hpp>
#include <dust/thread.hpp>
//======================================================================
#include <algorithm>
#include <dwmapi.h>
using dust::scoped_runtime_t;

scoped_runtime_t::scoped_runtime_t() {
#pragma warning(push)
#pragma warning(disable: 4995)
	//DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
#pragma warning(pop)
	//voodoo::prime_thread::spawn();
	voodoo::setup_dxgi();
	voodoo::setup_d3d_device();
}

scoped_runtime_t::~scoped_runtime_t() {

	std::for_each(threads_.begin(), threads_.end(), std::mem_fn(&std::thread::join));
	
	//voodoo::prime_thread::join();
	

#pragma warning(push)
#pragma warning(disable: 4995)
	//DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
#pragma warning(pop)
}

auto scoped_runtime_t::add_thread(std::function<void()> fn) -> void
{
	threads_.push_back(std::thread(fn));
}

auto scoped_runtime_t::add_context_thread(std::function<void()> fn) -> void
{
	threads_.push_back(voodoo::spawn_thread(fn));
}
