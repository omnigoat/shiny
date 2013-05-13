#ifndef SHINY_VOODOO_DEVICE_HPP
#define SHINY_VOODOO_DEVICE_HPP
//======================================================================
#include <thread>
#include <atomic>
#include <mutex>
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
		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;
		extern std::mutex immediate_context_mutex_;

		// this is the device context for this thread
		extern __declspec(thread) ID3D11DeviceContext* d3d_local_context_;

		inline auto is_prime_thread() -> bool { return d3d_local_context_ == d3d_immediate_context_; }

		struct scoped_async_immediate_context
		{
			scoped_async_immediate_context();
			~scoped_async_immediate_context();

			auto operator -> () const -> ID3D11DeviceContext*;
		};
	}

	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
