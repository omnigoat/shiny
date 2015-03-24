#pragma once

#include <shiny/shiny_fwd.hpp>

namespace sandbox
{
	struct plugin_t;
	struct application_t;




	struct plugin_t : atma::ref_counted
	{
		plugin_t(application_t*);
		virtual ~plugin_t();

		virtual auto input_bind(fooey::window_ptr const&) = 0;
		virtual auto input_update() = 0;
		virtual auto input_unbind() = 0;

		virtual auto gfx_setup(shiny::context_ptr const&) {}
		virtual auto gfx_teardown() {}
		virtual auto gfx_draw(shiny::scene_t const&) {}

	protected:
		auto dd_position_color() const -> shiny::data_declaration_t const* { return app_->dd_position_color; }

		auto vs_flat() const -> shiny::vertex_shader_cptr const&   { return app_->vs_flat; }
		auto fs_flat() const -> shiny::fragment_shader_cptr const& { return app_->fs_flat; }

		auto vb_cube() const -> shiny::vertex_buffer_cptr const&   { return app_->vb_cube; }
		auto ib_cube() const -> shiny::index_buffer_cptr const&    { return app_->ib_cube; }

	private:
		application_t* app_;
	};

	using plugin_ptr = atma::intrusive_ptr<plugin_t>;




	struct application_t
	{
		application_t();

		auto register_plugin(plugin_ptr const&) -> void;

		auto run() -> int;

	private:
		
	private:
		// data-declarations
		extern shiny::data_declaration_t const* dd_position_color;

		// shaders
		extern shiny::vertex_shader_cptr vs_flat;
		extern shiny::fragment_shader_cptr fs_flat;

		// geometry
		extern shiny::vertex_buffer_cptr vb_cube;
		extern shiny::index_buffer_cptr ib_cube;
	};
}
