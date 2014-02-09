#pragma once
//======================================================================
#include <dust/format.hpp>
#include <dust/adapter.hpp>
#include <dust/output.hpp>

#include <dust/platform/win32/d3d_fwd.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/events/resize.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>

#include <thread>
#include <mutex>
//======================================================================
namespace dust {
//======================================================================
	
	// forward declares
	struct vertex_declaration_t;
	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;
	struct vertex_shader_t;
	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;
	struct pixel_shader_t;
	typedef atma::intrusive_ptr<pixel_shader_t> pixel_shader_ptr;




	struct context_t : atma::ref_counted
	{
		context_t(runtime_t&, fooey::window_ptr const&, uint32_t adapter);
		~context_t();

		auto d3d_device() const -> platform::d3d_device_ptr { return d3d_device_; }

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint32_t output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear() -> void;
		auto signal_draw(vertex_declaration_t const&, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;

		auto signal_d3d_map(platform::d3d_buffer_ptr&, D3D11_MAPPED_SUBRESOURCE*, D3D11_MAP, uint32_t subresource, std::function<void(D3D11_MAPPED_SUBRESOURCE*)> const& = std::function<void(D3D11_MAPPED_SUBRESOURCE*)>()) -> void;
		auto signal_d3d_unmap(platform::d3d_buffer_ptr&, uint32_t subresource) -> void;
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr&, void const* data, uint32_t row_pitch, uint32_t depth_pitch) -> void;

		auto create_d3d_buffer(platform::d3d_buffer_ptr&, gpu_access_t, cpu_access_t, size_t data_size, void* data) -> void;

	private:
		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		auto setup_rendertarget(uint32_t width, uint32_t height) -> void;
		auto recreate_backbuffer() -> void;

		// these functions are called on a fooey thread
		auto on_resize(fooey::events::resize_t&) -> void;

	private:
		atma::thread::engine_t engine_;
		runtime_t& runtime_;

		// dxgi
		platform::dxgi_adapter_ptr dxgi_adapter_;
		platform::dxgi_swap_chain_ptr dxgi_swap_chain_;
		
		// d3d
		platform::d3d_device_ptr d3d_device_;
		platform::d3d_context_ptr d3d_immediate_context_;
		platform::d3d_context_ptr d3d_deferred_context_;
		atma::com_ptr<ID3D11RenderTargetView> d3d_render_target_;
		bool is_immediate_thread_;
		
		std::atomic_bool allow_present_;

		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// fullscreen
		bool fullscreen_;
		display_mode_t fullscreen_display_format_;
		platform::dxgi_output_ptr dxgi_output_;

		// windowed
		display_mode_t display_format_;
	};

	typedef atma::intrusive_ptr<context_t> context_ptr;

//======================================================================
} // namespace dust
//======================================================================
