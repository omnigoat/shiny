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

		auto gfx_setup(shiny::context_ptr const& ctx2) -> void override;
		auto gfx_draw(shiny::scene_t&) -> void override;
		
		auto main_setup() -> void override;

		auto result() const -> int override { return EXIT_SUCCESS; }

	private:
		shiny::context_ptr ctx;

		shiny::vertex_buffer_ptr vb;
		shiny::index_buffer_ptr ib;

		shiny::geometry_shader_ptr gs;



		shiny::buffer_ptr voxelbuf;
		shiny::resource_view_ptr voxelbuf_view;

		shiny::texture3d_ptr brickpool;
		shiny::buffer_ptr nodepool;
		shiny::resource_view_ptr nodepool_cview;

		using fragments_t = shiny::buffer_t::aligned_data_t<voxel_t>;
		fragments_t fragments;
	};

	using voxelization_plugin_ptr = atma::intrusive_ptr<voxelization_plugin_t>;;
}
