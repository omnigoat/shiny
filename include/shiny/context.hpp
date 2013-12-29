#ifndef SHINY_CONTEXT_HPP
#define SHINY_CONTEXT_HPP
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/format.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
#include <fooey/events/resize.hpp>
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
	
	namespace detail {
		struct context_t {
			//static 
		};
	}

	struct context_t : std::enable_shared_from_this<context_t>
	{
		context_t(fooey::window_ptr const&, uint32_t adapter);
		~context_t();

		auto toggle_fullscreen() -> void;

	private:
		auto setup_dxgi_and_d3d(uint32_t adapter) -> void;
		auto bind_to(fooey::window_ptr const&) -> void;
		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		
		auto on_resize(fooey::events::resize_t&) -> void;

		// dxgi
		voodoo::dxgi_adapter_ptr dxgi_adapter_;
		voodoo::dxgi_swap_chain_ptr dxgi_swap_chain_;
		voodoo::d3d_device_ptr d3d_device_;
		voodoo::d3d_context_ptr d3d_immediate_context_;

		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// fullscreen
		bool fullscreen_;
		display_mode_t fullscreen_display_format_;
		IDXGIOutput* dxgi_output_;

		// windowed
		display_mode_t display_format_;
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
