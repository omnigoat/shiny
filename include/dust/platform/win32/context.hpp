#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/element_format.hpp>
#include <dust/adapter.hpp>
#include <dust/output.hpp>

#include <dust/platform/win32/d3d_fwd.hpp>

#include <fooey/fooey_fwd.hpp>
#include <fooey/event_handler.hpp>

#include <atma/types.hpp>
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
		using map_callback_t = std::function<void(mapped_subresource_t&)>;

		~context_t();

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint32 output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear() -> void;
		auto signal_draw(vertex_declaration_t const*, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;
		auto signal_draw(index_buffer_ptr const&, vertex_declaration_t const*, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;
		auto signal_constant_buffer_upload(uint index, constant_buffer_cptr const&) -> void;
		auto signal_draw_scene(scene_t&) -> void;
		auto signal_update_constant_buffer(constant_buffer_ptr const&, uint data_size, void*) -> void;
		auto signal_update_constant_buffer(constant_buffer_ptr const&, atma::shared_memory_t const&) -> void;
		auto signal_upload_compute_shader(compute_shader_ptr const&) -> void;
		auto signal_upload_shader_resource(view_type_t, shader_resource2d_ptr const&) -> void;
		auto signal_compute_shader_dispatch(uint x, uint y, uint z) -> void;
		auto signal_map(resource_ptr const&, uint32 subresource, map_type_t, map_callback_t const&) -> void;

		auto signal_ps_upload_shader_resource(uint index, resource_ptr const&) -> void;


		// d3d-specific
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr const&, void const* data, uint32 row_pitch, uint32 depth_pitch) -> void;
		
		auto d3d_device() const -> platform::d3d_device_ptr const& { return d3d_device_; }
		auto d3d_immediate_context() const -> platform::d3d_context_ptr const& { return d3d_immediate_context_; }

	private:
		context_t(runtime_t&, fooey::window_ptr const&, uint32 adapter);

		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		auto setup_rendertarget(uint32 width, uint32 height) -> void;
		auto recreate_backbuffer() -> void;
		auto create_d3d_input_layout(vertex_shader_ptr const&, vertex_declaration_t const*) -> platform::d3d_input_layout_ptr;

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


		// cache for vertex-layouts
		std::map<std::tuple<vertex_shader_ptr, vertex_declaration_t const*>, platform::d3d_input_layout_ptr> cached_input_layouts_;



		friend auto create_context(runtime_t&, fooey::window_ptr const&, uint32 adapter) -> context_ptr;
	};

}

