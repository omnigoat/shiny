#ifndef SHINY_CONTEXT_HPP
#define SHINY_CONTEXT_HPP
//======================================================================
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/resources.hpp>
#include <shiny/format.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
#include <fooey/events/resize.hpp>
//======================================================================
#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>
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
	//   manages lifetime and threading of a swapchain
	//======================================================================
	struct context_t : atma::ref_counted<context_t>
	{
		context_t(fooey::window_ptr const&, uint32_t adapter);
		~context_t();

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint32_t output_index = primary_output) -> void;
		auto signal_present() -> void;

		//auto signal_upload_vertex_buffer_data(vertex_buffer_t const&, )

		auto create_d3d_buffer(voodoo::d3d_buffer_ptr&, gpu_access_t, cpu_access_t, uint32_t data_size, void* data) -> void;

	private:
		auto bind_events(fooey::window_ptr const&) -> void;
		auto signal_create_swapchain() -> void; 
		auto signal_setup_backbuffer() -> void;

		// these functions are called on a fooey thread
		auto on_resize(fooey::events::resize_t&) -> void;

	private:
		atma::thread::engine_t engine_;

		// dxgi
		voodoo::dxgi_adapter_ptr dxgi_adapter_;
		voodoo::dxgi_swap_chain_ptr dxgi_swap_chain_;
		
		// d3d
		voodoo::d3d_device_ptr d3d_device_;
		voodoo::d3d_context_ptr d3d_immediate_context_;
		atma::com_ptr<ID3D11RenderTargetView> d3d_render_target_;
		bool is_immediate_thread_;

		std::atomic_bool allow_present_;

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
