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
#include <atma/signal_hq.hpp>
//======================================================================
#include <thread>
#include <mutex>
//======================================================================
namespace shiny {
//======================================================================
	
	namespace detail {
		struct context_fns;
	}

	struct context_t;
	typedef atma::intrusive_ptr<context_t> context_ptr;

	//======================================================================
	// context_t
	// -----------
	//   manages lifetime and threading
	//======================================================================
	struct context_t : atma::ref_counted<context_t>
	{
		context_t(fooey::window_ptr const&, uint32_t adapter);
		~context_t();

		auto signal_fullscreen_toggle(uint32_t output_index = primary_output) -> void;

	private:
		auto setup_dxgi_and_d3d(uint32_t adapter) -> void;
		auto bind_to(fooey::window_ptr const&) -> void;
		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		
		auto on_resize(fooey::events::resize_t&) -> void;

	private:
		atma::signal_hq_t signal_hq_;

		// dxgi
		voodoo::dxgi_adapter_ptr dxgi_adapter_;
		voodoo::dxgi_swap_chain_ptr dxgi_swap_chain_;
		
		// d3d
		voodoo::d3d_device_ptr d3d_device_;
		voodoo::d3d_context_ptr d3d_immediate_context_;
		bool is_immediate_thread_;



		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// fullscreen
		bool fullscreen_;
		display_mode_t fullscreen_display_format_;
		voodoo::dxgi_output_ptr dxgi_output_;

		// windowed
		display_mode_t display_format_;
	};

	
	
	auto create_context(fooey::window_ptr const&, uint32_t adapter = primary_adapter) -> context_ptr;
	auto output_for_window(fooey::window_ptr const&) -> uint32_t;
	

//======================================================================
} // namespace shiny
//======================================================================
#endif
//======================================================================
