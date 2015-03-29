#pragma once

#include <sandbox/sandbox.hpp>

namespace sandbox
{
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
	};

	using voxelization_plugin_ptr = atma::intrusive_ptr<voxelization_plugin_t>;;
}
