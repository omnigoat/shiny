#ifndef SHINY_CONTEXT_HPP
#define SHINY_CONTEXT_HPP
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/format.hpp>
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
//======================================================================
	
	struct context_t : std::enable_shared_from_this<context_t>
	{
		context_t(fooey::window_ptr const&, uint32_t adapter);
		~context_t();

		auto toggle_fullscreen() -> void;

	private:
		auto bind_to(fooey::window_ptr const&) -> void;
		auto bind_events(fooey::window_ptr const&) -> void;
		auto enumerate_backbuffers() -> void;
		auto create_swapchain() -> void; 
		
		auto closest_fullscreen_backbuffer_mode(uint32_t width, uint32_t height)->display_mode_t;
		
		// dxgi
		IDXGIDevice1* dxgi_device_;
		IDXGIAdapter1* dxgi_adapter_;
		IDXGISwapChain* dxgi_swap_chain_;
		ID3D11Device* d3d_device_;

		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// fullscreen
		bool fullscreen_;
		uint32_t fullscreen_width_, fullscreen_height_;
		IDXGIOutput* dxgi_output_;

		typedef std::vector<display_mode_t> display_modes_t;
		display_modes_t backbuffer_display_modes_;
		uint32_t width_, height_;
	};

	typedef std::shared_ptr<context_t> context_ptr;
	typedef std::weak_ptr<context_t> context_wptr;
	
	auto create_context(fooey::window_ptr const&, uint32_t adapter = primary_adapter) -> context_ptr;
	
	
	auto signal_fullscreen_toggle(context_ptr const& context) -> void;

//======================================================================
} // namespace shiny
//======================================================================
#endif
//======================================================================
