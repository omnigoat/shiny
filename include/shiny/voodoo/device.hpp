#ifndef SHINY_VOODOO_DEVICE_HPP
#define SHINY_VOODOO_DEVICE_HPP
//======================================================================
#include <fooey/widgets/window.hpp>
//======================================================================
#include <atma/intrusive_ptr.hpp>
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
	namespace detail
	{
		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;
		extern std::mutex immediate_context_mutex_;

		// this is the device context for this thread
		extern __declspec(thread) ID3D11DeviceContext* d3d_local_context_;

		inline auto is_prime_thread() -> bool { return d3d_local_context_ == d3d_immediate_context_; }

		struct scoped_async_immediate_context_t
		{
			scoped_async_immediate_context_t();
			~scoped_async_immediate_context_t();

			auto operator -> () const -> ID3D11DeviceContext*;
		};
	}

	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;
	
	struct context_t : atma::ref_counted
	{
		context_t(fooey::window_ptr const&);
		~context_t();

	private:
		fooey::window_ptr window_;
		uint32_t on_resize_handle_;
		IDXGISwapChain* dxgi_swap_chain_;
	};
	typedef atma::intrusive_ptr<context_t> context_ptr;


	auto create_context(fooey::window_ptr const&, uint32_t width, uint32_t height) -> context_ptr;

//======================================================================
} // namespace voodoo

	using voodoo::create_context;

} // namespace shiny
//======================================================================
#endif
//======================================================================
