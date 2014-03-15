#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/surface_format.hpp>
#include <dust/adapter.hpp>
#include <dust/output.hpp>

#include <dust/platform/win32/d3d_fwd.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/events/resize.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>
#include <atma/shared_memory.hpp>

#include <thread>
#include <mutex>
//======================================================================
namespace dust {
//======================================================================
	
	struct context_t : atma::ref_counted
	{
		~context_t();

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint32 output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear() -> void;
		auto signal_draw(vertex_declaration_t const&, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;
		auto signal_draw(index_buffer_ptr const&, vertex_declaration_t const&, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;
		auto signal_constant_buffer_upload(uint index, constant_buffer_ptr const&) -> void;
		auto signal_draw_scene(scene_t&) -> void;
		auto signal_update_constant_buffer(constant_buffer_ptr const&, uint data_size, void*) -> void;
		auto signal_update_constant_buffer(constant_buffer_ptr const&, atma::shared_memory const&) -> void;

		// d3d-specific
		auto signal_d3d_map(platform::d3d_buffer_ptr&, D3D11_MAP, uint32 subresource, std::function<void(D3D11_MAPPED_SUBRESOURCE*)> const& = std::function<void(D3D11_MAPPED_SUBRESOURCE*)>()) -> void;
		auto signal_d3d_unmap(platform::d3d_buffer_ptr&, uint32 subresource) -> void;
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr&, void const* data, uint32 row_pitch, uint32 depth_pitch) -> void;
		
		auto create_d3d_buffer(platform::d3d_buffer_ptr&, buffer_type_t, gpu_access_t, cpu_access_t, size_t data_size, void* data) -> void;
		//auto create_d3d_texture2d(platform::d3d_texture2d_ptr&, texture_usage_t, surface_format_t, uint width, uint height) -> void;

		auto d3d_device() const -> platform::d3d_device_ptr { return d3d_device_; }

	private:
		context_t(runtime_t&, fooey::window_ptr const&, uint32 adapter);

		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		auto setup_rendertarget(uint32 width, uint32 height) -> void;
		auto recreate_backbuffer() -> void;
		
		auto pull_display_format(display_mode_t&, DXGI_SWAP_CHAIN_DESC&) -> void;
		auto push_display_format(DXGI_MODE_DESC&, display_mode_t const&) -> void;

		auto update_display_mode() -> void;

		// these functions are called on a fooey thread
		auto on_resize(fooey::events::resize_t&) -> void;

	private:
		atma::thread::engine_t engine_;
		runtime_t& runtime_;

		platform::dxgi_adapter_ptr dxgi_adapter_;
		platform::dxgi_swap_chain_ptr dxgi_swap_chain_;
		platform::dxgi_output_ptr dxgi_output_;

		platform::d3d_device_ptr d3d_device_;
		platform::d3d_context_ptr d3d_immediate_context_;
		platform::d3d_context_ptr d3d_deferred_context_;
		platform::d3d_render_target_view_ptr d3d_render_target_;
		platform::d3d_depth_stencil_buffer_ptr d3d_depth_stencil_;
		platform::d3d_texture2d_ptr d3d_depth_stencil_buffer_;

		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// our current display format (windowed), current display format (fullscreen),
		// which display mode we're currently at, the requested modes for both windowed
		// and fullscreen, and which mode we should transition to (changing between
		// windowed/fullscreen takes precedence over windowed/windowed size changes).
		display_mode_t windowed_display_mode_, fullscreen_display_mode_;
		display_mode_t* current_display_mode_;
		display_mode_t requested_windowed_display_mode_, requested_fullscreen_display_mode_;
		display_mode_t* requested_display_mode_;

		friend auto create_context(runtime_t&, fooey::window_ptr const&, uint32 adapter) -> context_ptr;
	};

//======================================================================
} // namespace dust
//======================================================================
