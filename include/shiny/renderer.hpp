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
		virtual auto signal_draw_scene(scene_t&) -> void;
		virtual auto signal(atma::thread::engine_t::queue_t::batch_t&) -> void;
		virtual auto signal_copy_buffer(resource_ptr const&, resource_cptr const&) -> void;
		virtual auto signal_fullscreen_toggle(uint output_index = primary_output) -> void;
		virtual auto signal_clear(atma::math::vector4f const&) -> void;



	private:
		renderer2_t(runtime_t&, fooey::window_ptr const&, uint adapter);

		runtime_t& runtime_;

		virtual auto backbuffer_render_target() -> resource_view_ptr const& = 0;
		virtual auto backbuffer_depth_stencil() -> resource_view_ptr const& = 0;

	private:
		friend struct atma::enable_default_intrusive_ptr_make;
	};
}
#endif

