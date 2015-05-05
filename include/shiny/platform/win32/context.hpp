#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/element_format.hpp>
#include <shiny/adapter.hpp>
#include <shiny/output.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/generic_buffer.hpp>
#include <shiny/geometry_shader.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/draw_state.hpp>
#include <shiny/draw.hpp>
#include <shiny/rendertarget_clear.hpp>

#include <shiny/platform/win32/d3d_fwd.hpp>
#include <shiny/platform/win32/blender.hpp>

#include <fooey/fooey_fwd.hpp>
#include <fooey/event_handler.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>
#include <atma/shared_memory.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/hash.hpp>

#include <thread>
#include <mutex>
#include <unordered_map>


namespace shiny
{
	namespace constant_buffer_index
	{
		uint const user = 3;
	}

	struct context_t : atma::ref_counted
	{
		using map_callback_t = std::function<void(mapped_subresource_t&)>;

		~context_t();

		auto runtime() -> runtime_t& { return runtime_; }
		auto runtime() const -> runtime_t const& { return runtime_; }

		auto make_generic_buffer(resource_usage_mask_t const&, resource_storage_t, size_t stride, uint elements, void const* data, uint data_elemcount) -> generic_buffer_ptr;
		auto make_generic_buffer(resource_usage_mask_t const&, resource_storage_t, size_t stride, uint elements) -> generic_buffer_ptr;
		auto make_blender(blend_state_t const&) -> blender_ptr;


		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear(atma::math::vector4f const&) -> void;
		auto signal_draw_scene(scene_t&) -> void;

		auto immediate_clear(rendertarget_clear_t const&) -> void;


		// pipeline-setup-stage
		auto immediate_pipeline_reset() -> void;


		// "resources stage"
		auto signal_res_map(resource_ptr const&, uint subresource, map_type_t, map_callback_t const&) -> void;
		auto signal_res_update(constant_buffer_ptr const&, uint data_size, void*) -> void;
		auto signal_res_update(constant_buffer_ptr const&, atma::shared_memory_t const&) -> void;


		// input-assembly-stage
		auto immediate_ia_set_data_declaration(data_declaration_t const*) -> void;
		auto immediate_ia_set_vertex_buffer(vertex_buffer_cptr const&) -> void;
		auto immediate_ia_set_index_buffer(index_buffer_cptr const&) -> void;
		auto immediate_ia_set_topology(topology_t) -> void;


		// geometry-stage
		auto immediate_gs_set_geometry_shader(geometry_shader_cptr const&) -> void;


		// vertex-stage
		auto immediate_vs_set_vertex_shader(vertex_shader_cptr const&) -> void;
		auto immediate_vs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_vs_set_resources(bound_resources_t const&) -> void;


		// fragment-stage
		auto immediate_fs_set_fragment_shader(fragment_shader_cptr const&) -> void;
		auto immediate_fs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_fs_set_resources(bound_resources_t const&) -> void;


		// output-merger-stage
		auto immediate_om_set_blending(blender_cptr const&) -> void;


		// draw
		auto immediate_draw_set_range(draw_range_t const&) -> void;
		auto immediate_draw() -> void;


		// compute-stage
		auto signal_cs_set(compute_shader_ptr const&) -> void;
		auto signal_cs_upload_constant_buffer(uint index, constant_buffer_cptr const&) -> void;
		auto signal_cs_dispatch(uint x, uint y, uint z) -> void;




		// d3d-specific
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr const&, void const* data, uint row_pitch, uint depth_pitch) -> void;
		
		auto d3d_device() const -> platform::d3d_device_ptr const& { return d3d_device_; }
		auto d3d_immediate_context() const -> platform::d3d_context_ptr const& { return d3d_immediate_context_; }


	private:
		context_t(runtime_t&, fooey::window_ptr const&, uint adapter);

		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		auto setup_rendertarget(uint width, uint height) -> void;
		auto recreate_backbuffer() -> void;
		auto create_d3d_input_layout(vertex_shader_cptr const&, data_declaration_t const*) -> platform::d3d_input_layout_ptr;

		auto setup_debug_geometry() -> void;
		auto setup_debug_shaders() -> void;

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
		platform::d3d_render_target_view_ptr d3d_render_target_;
		platform::d3d_depth_stencil_buffer_ptr d3d_depth_stencil_;
		platform::d3d_texture2d_ptr d3d_depth_stencil_buffer_;

		data_declaration_t const* data_declaration_;
		index_buffer_cptr index_buffer_;
		vertex_shader_cptr vertex_shader_;
		draw_range_t draw_range_;

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

		// blenders
		std::unordered_map<blend_state_t, blender_ptr, atma::std_hash_functor_adaptor_t> cached_blenders_;

		// cache for vertex-layouts
		std::map<std::tuple<vertex_shader_cptr, data_declaration_t const*>, platform::d3d_input_layout_ptr> cached_input_layouts_;


		// debug stuff
		vertex_buffer_ptr debug_vertices_;
		index_buffer_ptr debug_indices_;


	private:
		friend auto create_context(runtime_t&, fooey::window_ptr const&, uint adapter) -> context_ptr;
	};

}

