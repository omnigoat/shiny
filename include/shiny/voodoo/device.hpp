#ifndef SHINY_VOODOO_DEVICE_HPP
#define SHINY_VOODOO_DEVICE_HPP
//======================================================================
#include <thread>
#include <atomic>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace voodoo {
//======================================================================

	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail {
		// the prime thread is the thread upon which @d3d_immediate_context_ resides.
		extern std::thread prime_thread_;
		extern std::atomic_bool prime_thread_running_;

		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;
	}

	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
