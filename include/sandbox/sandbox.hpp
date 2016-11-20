#pragma once

#include <sandbox/plugin.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/runtime.hpp>

#include <fooey/fooey_fwd.hpp>

#include <rose/runtime.hpp>


namespace sandbox
{
	struct application_t
	{
		application_t(rose::runtime_t*);

		auto register_plugin(plugin_ptr const&) -> void;

		auto run() -> int;

		auto vfs() -> lion::vfs_t& { return vfs_; }

	private:
		fooey::renderer_ptr window_renderer;
		fooey::window_ptr window;

		shiny::runtime_t runtime;
		shiny::renderer_ptr ctx;

		std::vector<plugin_ptr> plugins_;

		rose::runtime_t* rose_runtime_;

		lion::vfs_t vfs_;

	private:
		// data-declarations
		shiny::data_declaration_t const* dd_position;
		shiny::data_declaration_t const* dd_position_color;

		// shaders
		shiny::vertex_shader_ptr vs_flat;
		shiny::fragment_shader_ptr fs_flat;

		// geometry
		static float cube_vertices[];
		static uint16 cube_indices[];
		shiny::vertex_buffer_cptr vb_cube;
		shiny::index_buffer_cptr ib_cube;

		friend struct plugin_t;
	};
}
