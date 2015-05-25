#pragma once

#include <sandbox/sandbox.hpp>

#include <shiny/buffer.hpp>

namespace sandbox
{
	struct voxel_t
	{
		uint32 morton;
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
		voxelization_plugin_t(application_t* app)
			: plugin_t{app}
		{}

		auto gfx_setup(shiny::context_ptr const&) -> void override;
		auto gfx_ctx_draw(shiny::context_ptr const&) -> void override;
		auto gfx_draw(shiny::scene_t&) -> void override;
		
		auto main_setup() -> void override;
		auto setup_voxelization() -> void;
		auto setup_svo() -> void;
		auto setup_rendering() -> void;

		auto result() const -> int override { return EXIT_SUCCESS; }

	private:
		shiny::context_ptr ctx;

		shiny::vertex_buffer_ptr vb;
		shiny::index_buffer_ptr ib;

		shiny::geometry_shader_ptr gs;
		shiny::compute_shader_ptr cs_clear;

	private:
		// fragments
		using fragments_t = shiny::buffer_t::aligned_data_t<voxel_t>;
		fragments_t fragments;
		shiny::buffer_ptr voxelbuf;
		shiny::resource_view_ptr voxelbuf_view;

		// svo
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
		shiny::fragment_shader_ptr fs_voxels;
	};

	using voxelization_plugin_ptr = atma::intrusive_ptr<voxelization_plugin_t>;;
}
