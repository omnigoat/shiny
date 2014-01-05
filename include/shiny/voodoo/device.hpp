#ifndef SHINY_VOODOO_DEVICE_HPP
#define SHINY_VOODOO_DEVICE_HPP
//======================================================================
#include <shiny/format.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
//======================================================================
#include <atma/com_ptr.hpp>
//======================================================================
#include <thread>
#include <atomic>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
	
	uint32_t const primary_adapter = 0;
	uint32_t const primary_output = 0;

namespace voodoo {
//======================================================================
	
	// dxgi
	typedef atma::com_ptr<IDXGIFactory1> dxgi_factory_ptr;
	typedef atma::com_ptr<IDXGIAdapter1> dxgi_adapter_ptr;
	typedef atma::com_ptr<IDXGIOutput> dxgi_output_ptr;
	typedef atma::com_ptr<IDXGISwapChain> dxgi_swap_chain_ptr;
	
	// d3d
	typedef atma::com_ptr<ID3D11Device> d3d_device_ptr;
	typedef atma::com_ptr<ID3D11DeviceContext> d3d_context_ptr;
	
	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail
	{
		
		// this is the device context for this thread
		//extern __declspec(thread) ID3D11DeviceContext* d3d_local_context_;


		auto output_for_window(fooey::window_ptr const&) -> atma::com_ptr<IDXGIOutput>;

		struct scoped_async_immediate_context_t
		{
			scoped_async_immediate_context_t();
			~scoped_async_immediate_context_t();

			auto operator -> () const -> atma::com_ptr<ID3D11DeviceContext> const&;
		};
	}

	auto dxgi_factory() -> dxgi_factory_ptr;

	auto dxgi_and_d3d_at(uint32_t adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>;
	auto adapter_and_output(uint32_t adapter, uint32_t output) -> std::tuple<dxgi_adapter_ptr, dxgi_output_ptr>;
	auto output_at(dxgi_adapter_ptr const&, uint32_t output_index) -> dxgi_output_ptr;
	auto closest_matching_format(dxgi_output_ptr const&, uint32_t width, uint32_t height) -> display_mode_t;

	auto setup_dxgi() -> void;
	auto teardown_dxgi() -> void;
	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;
	
//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
