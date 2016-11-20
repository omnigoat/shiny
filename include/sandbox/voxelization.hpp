#pragma once

#include <sandbox/sandbox.hpp>

#include <shiny/buffer.hpp>

namespace sandbox
{
	struct voxel_t
	{
		uint32 morton;

	};

	struct node_t
	{
		node_t()
			: children_offset{}
			, brick_idx{}
		{
		}

		uint32 children_offset;
		uint32 brick_idx;
	};

	inline auto operator < (voxel_t const& lhs, voxel_t const& rhs) -> bool
	{
		return lhs.morton < rhs.morton;
	}

	inline auto operator == (voxel_t const& lhs, voxel_t const& rhs) -> bool
	{
		return lhs.morton == rhs.morton;
	}

	struct voxelization_plugin_t : plugin_t
	{
		voxelization_plugin_t() = delete;
		voxelization_plugin_t(application_t*);

		auto gfx_setup(shiny::renderer_ptr const&) -> void override;
		auto gfx_ctx_draw(shiny::renderer_ptr const&) -> void override;
		auto gfx_draw(shiny::scene_t&) -> void override;
		
		auto main_setup() -> void override;
		auto setup_voxelization() -> void;
		auto setup_svo() -> void;
		auto setup_rendering() -> void;

		auto result() const -> int override { return EXIT_SUCCESS; }

	private:
		shiny::renderer_ptr rndr;

		shiny::vertex_buffer_ptr vb;
		shiny::index_buffer_ptr ib;

		shiny::geometry_shader_ptr gs;
		shiny::compute_shader_ptr cs_clear;

	private:
		// voxelization
		using fragments_t = shiny::buffer_t::aligned_data_t<voxel_t>;
		
		shiny::texture2d_ptr render_texture, depth_stencil_texture;
		//shiny::render_target_ptr render_target;
		//pepper::draw_target_ptr draw_target;
		shiny::buffer_ptr fragments_buf;
		uint fragments_count;
		shiny::resource_view_ptr fragments_view;
		shiny::resource_view_ptr fragments_srv_view;
		shiny::vertex_shader_ptr vs_voxelize;
		shiny::geometry_shader_ptr gs_voxelize;
		shiny::fragment_shader_handle fs_voxelize;
#if 0
		fragments_t fragments;
		shiny::buffer_ptr voxelbuf;
		shiny::resource_view_ptr voxelbuf_view;
#endif
		
		lion::asset_library_t library_;
		

		// svo
		using nodes_t = std::vector<node_t>;
		nodes_t nodes;
		shiny::texture3d_ptr brickcache;
		shiny::buffer_ptr nodecache, countbuf;
		shiny::resource_view_ptr brickcache_view, nodecache_view, countbuf_view;
		shiny::compute_shader_ptr cs_mark, cs_allocate, cs_write_fragments;
		shiny::resource_view_ptr nodecache_input_view, brickcache_input_view;

		// debug
		shiny::buffer_ptr stb;

	private:
		// rendering
		shiny::vertex_buffer_ptr vb_quad;
		shiny::vertex_shader_ptr vs_voxels;
		shiny::fragment_shader_handle fs_voxels;
	};

	using voxelization_plugin_ptr = atma::intrusive_ptr<voxelization_plugin_t>;;
}
