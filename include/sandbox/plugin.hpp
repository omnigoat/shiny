#pragma once

#include <shiny/shiny_fwd.hpp>

#include <fooey/fooey_fwd.hpp>

namespace sandbox
{
	struct plugin_t;
	struct application_t;

	struct plugin_t : atma::ref_counted
	{
		plugin_t(application_t* app)
			: app_(app)
		{}

		virtual ~plugin_t() {}

		virtual auto input_bind(fooey::window_ptr const&) -> void {}
		virtual auto input_update() -> void {}
		virtual auto input_unbind() -> void {}

		virtual auto gfx_setup(shiny::context_ptr const&) -> void {}
		virtual auto gfx_draw(shiny::scene_t&) -> void {}
		virtual auto gfx_ctx_draw(shiny::context_ptr const&) -> void {}
		virtual auto gfx_teardown() -> void {}

		virtual auto main_setup() -> void {}

		virtual auto result() const -> int = 0;

	protected:
#if 0
		auto dd_position_color() const -> shiny::data_declaration_t const* { return app_->dd_position_color; }

		auto vs_flat() const -> shiny::vertex_shader_cptr const&   { return app_->vs_flat; }
		auto fs_flat() const -> shiny::fragment_shader_cptr const& { return app_->fs_flat; }

		auto vb_cube() const -> shiny::vertex_buffer_cptr const&   { return app_->vb_cube; }
		auto ib_cube() const -> shiny::index_buffer_cptr const&    { return app_->ib_cube; }
#endif
		auto dd_position() const -> shiny::data_declaration_t const*;
		auto dd_position_color() const -> shiny::data_declaration_t const*;

		auto cube_vertices() const -> float const*;
		auto cube_indices() const -> uint16 const*;

		auto vs_flat() const -> shiny::vertex_shader_ptr const&;
		auto fs_flat() const -> shiny::fragment_shader_ptr const&;

	private:
		application_t* app_;
	};

	using plugin_ptr = atma::intrusive_ptr<plugin_t>;
}
