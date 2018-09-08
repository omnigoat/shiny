#pragma once

#include <shiny/shiny_fwd.hpp>

#include <fooey/fooey_fwd.hpp>

#include <atma/config/platform.hpp>

namespace shiny
{
	uint32 const primary_adapter = 0;

	auto create_context(runtime_t&, fooey::window_ptr const&, uint32 adapter = primary_adapter) -> renderer_ptr;
}

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny/platform/dx11/renderer.hpp>
#endif




// PIMPL
namespace shiny
{
	struct renderer_impl_t
	{
		virtual auto initialize(runtime_t&, uint adapter) -> bool;
		virtual auto device_make_buffer(resource_type_t, resource_usage_mask_t, resource_storage_t, buffer_dimensions_t, buffer_data_t) -> resource_ptr;
	};

	using renderer_impl_ptr = std::unique_ptr<renderer_impl_t>;
}

#if 1
namespace shiny
{
	struct renderer2_t : atma::ref_counted
	{
		auto runtime() -> runtime_t& { return runtime_; }
		auto runtime() const -> runtime_t const& { return runtime_; }

		// executed asynchonrously
		virtual auto signal_block() -> void = 0;
		virtual auto signal_present() -> void = 0;
		virtual auto signal_draw_scene(scene_t&) -> void {}
		virtual auto signal_copy_buffer(resource_ptr const&, resource_cptr const&) -> void {}
		virtual auto signal_fullscreen_toggle(uint output_index = primary_output) -> void {}
		virtual auto signal_clear(atma::math::vector4f const&) -> void = 0;


	public: // resource creation
		auto make_buffer(resource_type_t, resource_usage_mask_t, resource_storage_t, buffer_dimensions_t, buffer_data_t) -> buffer_ptr;
		auto make_constant_buffer(void const* data, size_t data_size) -> constant_buffer_ptr;
		auto make_index_buffer(resource_storage_t, format_t, uint indexcount, void const* data, uint datacount = 0) -> index_buffer_ptr;
		auto make_vertex_buffer(resource_storage_t, data_declaration_t const*, size_t bufcount, void const* data, size_t datacount = 0) -> vertex_buffer_ptr;
		auto make_texture2d(resource_usage_mask_t, resource_storage_t, format_t, uint width, uint height, uint mips) -> texture2d_ptr;
		auto make_texture3d(resource_usage_mask_t, resource_storage_t, format_t, uint width, uint height, uint depth, uint mips) -> texture3d_ptr;
		auto make_vertex_shader(lion::path_t const&, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr;

		template <typename T>
		auto make_constant_buffer_for(T const& t) -> constant_buffer_ptr
			{ return make_constant_buffer(&t, sizeof(t)); }

		template <typename T>
		auto make_vertex_buffer(resource_storage_t storage, data_declaration_t const* dd, atma::vector<T> const& x) -> vertex_buffer_ptr
			{ ATMA_ASSERT(dd->stride() == sizeof(T), "invalid sizes for vertex-buffer shortcut creation");
			  return make_vertex_buffer(storage, dd, x.size(), x.data(), x.size()); }

		
	public: // pipeline state
		// pipeline-setup-stage
		auto immediate_draw_pipeline_reset() -> void;
		auto immediate_compute_pipeline_reset() -> void;

		// resource-stage
		auto signal_rs_upload(resource_ptr const&, buffer_data_t const&) -> void;
		auto signal_rs_upload(resource_ptr const&, uint subresource, buffer_data_t const&) -> void;
		template <typename T> auto signal_rs_upload(resource_ptr const&, T const&) -> void;
		//auto signal_rs_map(resource_ptr const&, uint subresource, map_type_t, map_callback_t const&) -> void;

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

	private:
		renderer2_t(runtime_t&, fooey::window_ptr const&, uint adapter);
		virtual auto backbuffer_render_target() -> resource_view_ptr const& = 0;
		virtual auto backbuffer_depth_stencil() -> resource_view_ptr const& = 0;

		runtime_t& runtime_;
		renderer_impl_ptr impl_;
		

	private:
		friend struct atma::enable_intrusive_ptr_make;
	};
}
#endif

