#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/format.hpp>
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
#include <shiny/depth_state.hpp>

#include <shiny/platform/dx11/d3d_fwd.hpp>
#include <shiny/platform/dx11/blender.hpp>

#include <fooey/fooey_fwd.hpp>
#include <fooey/event_handler.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>
#include <atma/shared_memory.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/hash.hpp>

#include <deque>
#include <thread>
#include <mutex>
#include <unordered_map>


namespace shiny
{
	namespace constant_buffer_index
	{
		uint const user = 3;
	}

	enum class renderer_stage_t
	{
		init,
		resource_upload,
		render,
		//present,
		//teardown,
	};

	struct renderer_t : atma::ref_counted
	{
		using map_callback_t = std::function<void(mapped_subresource_t&)>;

		~renderer_t();

		auto runtime() -> runtime_t& { return runtime_; }
		auto runtime() const -> runtime_t const& { return runtime_; }

		auto backbuffer_render_target() -> resource_view_ptr const&;
		auto backbuffer_depth_stencil() -> resource_view_ptr const&;

		auto make_generic_buffer(resource_usage_mask_t const&, resource_storage_t, size_t stride, uint elements, void const* data, uint data_elemcount) -> generic_buffer_ptr;
		auto make_generic_buffer(resource_usage_mask_t const&, resource_storage_t, size_t stride, uint elements) -> generic_buffer_ptr;

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear(atma::math::vector4f const&) -> void;
		auto signal_draw_scene(scene_t&) -> void;
		auto signal(atma::thread::engine_t::queue_t::batch_t&) -> void;
		auto signal_stage_change(renderer_stage_t) -> void;
		auto signal_copy_buffer(resource_ptr const&, resource_cptr const&) -> void;

		auto immediate_set_stage(renderer_stage_t) -> void;

		auto immediate_clear(rendertarget_clear_t const&) -> void;

		// make functions
		// these are threadsafe
		//auto make_render_target_view(resource_cptr const&) -> render_target_view_ptr;


		// pipeline-setup-stage
		auto immediate_draw_pipeline_reset() -> void;
		auto immediate_compute_pipeline_reset() -> void;

		// resource-stage
		auto signal_rs_upload(resource_ptr const&, buffer_data_t const&) -> void;
		auto signal_rs_upload(resource_ptr const&, uint subresource, buffer_data_t const&) -> void;
		template <typename T> auto signal_rs_upload(resource_ptr const&, T const&) -> void;
		auto signal_rs_map(resource_ptr const&, uint subresource, map_type_t, map_callback_t const&) -> void;

		// input-assembly-stage
		auto immediate_ia_set_data_declaration(data_declaration_t const*) -> void;
		auto immediate_ia_set_vertex_buffer(vertex_buffer_cptr const&) -> void;
		auto immediate_ia_set_index_buffer(index_buffer_cptr const&) -> void;
		auto immediate_ia_set_topology(topology_t) -> void;

		// vertex-stage
		auto immediate_vs_set_vertex_shader(vertex_shader_cptr const&) -> void;
		auto immediate_vs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_vs_set_input_views(bound_input_views_t const&) -> void;

		// geometry-stage
		auto immediate_gs_set_geometry_shader(geometry_shader_cptr const&) -> void;
		auto immediate_gs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_gs_set_input_views(bound_input_views_t const&) -> void;

		// fragment-stage
		auto immediate_fs_set_fragment_shader(fragment_shader_handle const&) -> void;
		auto immediate_fs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_fs_set_input_views(bound_input_views_t const&) -> void;
		auto immediate_fs_set_compute_views(bound_compute_views_t const&) -> void;

		// output-merger-stage
		auto immediate_om_set_render_target(resource_view_ptr const&) -> void;
		auto immediate_om_set_depth_stencil(resource_view_ptr const&) -> void;
		auto immediate_om_set_blending(blender_cptr const&) -> void;

		// draw
		auto immediate_draw_set_range(draw_range_t const&) -> void;
		auto immediate_draw() -> void;


		// compute-stage
		auto immediate_cs_set_constant_buffers(bound_constant_buffers_t const&) -> void;
		auto immediate_cs_set_input_views(bound_resource_views_t const&) -> void;
		auto immediate_cs_set_compute_views(bound_resource_views_t const&) -> void;
		auto immediate_cs_set_compute_shader(compute_shader_cptr const&) -> void;
		auto immediate_compute(uint x, uint y, uint z) -> void;




		// d3d-specific
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr const&, void const* data, uint row_pitch, uint depth_pitch) -> void;
		
		auto d3d_device() const -> platform::d3d_device_ptr const& { return d3d_device_; }
		auto d3d_immediate_context() const -> platform::d3d_renderer_ptr const& { return d3d_immediate_context_; }


	private:
		renderer_t(runtime_t&, fooey::window_ptr const&, uint adapter);

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

		
		//auto make_depth_stencil(depth_stencil_state_t const&) -> 

		// these functions are called on a fooey thread
		auto on_resize(fooey::events::resize_t&) -> void;


	private:
		using input_layouts_t = std::map<std::tuple<data_declaration_t const*, vertex_shader_cptr>, platform::d3d_input_layout_ptr>;
		using blend_states_t = std::unordered_map<blend_state_t, platform::d3d_blend_state_ptr, atma::std_hash_functor_adaptor_t>;
		using depth_stencil_states_t = std::unordered_map<depth_stencil_state_t, platform::d3d_depth_stencil_state_ptr, atma::std_hash_functor_adaptor_t>;

		auto get_d3d_input_layout(data_declaration_t const*, vertex_shader_cptr const&) -> platform::d3d_input_layout_ptr const&;
		auto get_d3d_blend(blend_state_t const&) -> platform::d3d_blend_state_ptr const&;
		auto get_d3d_depth_stencil(depth_stencil_state_t const&) -> platform::d3d_depth_stencil_state_ptr const&;

		input_layouts_t        built_input_layouts_;
		blend_states_t         built_blend_states_;
		depth_stencil_states_t built_depth_stencil_states_;

	private:
		//struct frame_resources_t
		//{
			lion::asset_library_t library;

			// compute pipeline
			bound_constant_buffers_t cs_cbs_;
			bound_resource_views_t cs_uavs_;
			bound_resource_views_t cs_srvs_;
			compute_shader_cptr cs_shader_;

			// draw pipeline
			data_declaration_t const* ia_dd_;
			index_buffer_cptr         ia_ib_;
			vertex_buffer_cptr        ia_vb_;
			vertex_shader_cptr        vs_shader_;
			bound_constant_buffers_t  vs_cbs_;
			bound_resource_views_t    vs_srvs_;
			geometry_shader_cptr      gs_shader_;
			bound_constant_buffers_t  gs_cbs_;
			fragment_shader_handle    fs_shader_;
			bound_constant_buffers_t  fs_cbs_;
			bound_resource_views_t    fs_srvs_;
			bound_resource_views_t    fs_uavs_;
			bound_resource_views_t    om_rtvs_;
			depth_stencil_state_t     om_depth_stencil_;
			draw_range_t              draw_range_;

			// render-targets & depth-stencil
			resource_view_ptr current_render_target_view_[4];
			resource_view_ptr current_depth_stencil_view_;
		//};
		//
		//std::deque<frame_resources_t> frames_;

	private:
		runtime_t& runtime_;
		atma::thread::engine_t engine_;

		platform::dxgi_adapter_ptr dxgi_adapter_;
		platform::d3d_device_ptr   d3d_device_;
		platform::d3d_renderer_ptr  d3d_immediate_context_;


	private:
		// stage
		renderer_stage_t stage_;

		// swap-chain
		platform::dxgi_output_ptr        dxgi_output_;
		platform::dxgi_swap_chain_ptr    dxgi_swap_chain_;
		platform::d3d_texture2d_ptr      d3d_backbuffer_;
		
		texture2d_ptr                    backbuffer_texture_;
		resource_view_ptr                backbuffer_view_;
		texture2d_ptr                    default_depth_stencil_texture_;
		resource_view_ptr                default_depth_stencil_view_;

		
		
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

		// debug stuff
		vertex_buffer_ptr debug_vertices_;
		index_buffer_ptr debug_indices_;


	private:
		friend struct atma::enable_default_intrusive_ptr_make;
	};


	template <typename T>
	inline auto renderer_t::signal_rs_upload(resource_ptr const& res, T const& t) -> void
	{
		signal_rs_upload(res, buffer_data_t{&t, sizeof(t)});
	}
}

