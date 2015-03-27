#pragma once

#include <sandbox/plugin.hpp>

#include <shiny/shiny_fwd.hpp>
#include <shiny/runtime.hpp>

#include <fooey/fooey_fwd.hpp>

namespace sandbox
{
	struct application_t
	{
		application_t();

		auto register_plugin(plugin_ptr const&) -> void;

		auto run() -> int;

	private:
		
	private:
		fooey::renderer_ptr window_renderer;
		fooey::window_ptr window;

		shiny::runtime_t runtime;
		shiny::context_ptr ctx;

		std::vector<plugin_ptr> plugins_;

	private:
		// data-declarations
		shiny::data_declaration_t const* dd_position_color;

		// shaders
		shiny::vertex_shader_cptr vs_flat;
		shiny::fragment_shader_cptr fs_flat;

		// geometry
		shiny::vertex_buffer_cptr vb_cube;
		shiny::index_buffer_cptr ib_cube;
	};
}
