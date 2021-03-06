#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/draw.hpp>
#include <shiny/camera.hpp>
#include <shiny/draw_target.hpp>

#include <shiny/rendertarget_clear.hpp>

#include <atma/types.hpp>
#include <atma/thread/engine.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/platform/allocation.hpp>


namespace shiny
{
	struct scene_t
	{
		scene_t(scene_t const&) {}
		scene_t(scene_t&&);
		scene_t(renderer_ptr const&, draw_target_t const&, camera_t const&, rendertarget_clear_t const&);
		scene_t(renderer_ptr const&, draw_target_t const&);
		scene_t(renderer_ptr const&, camera_t const&, rendertarget_clear_t const&);

		auto scene_constant_buffer() const -> constant_buffer_ptr const& { return scene_constant_buffer_; }

		auto renderer() const -> renderer_ptr const& { return rndr_; }
		auto draw_target() const -> draw_target_t const& { return draw_target_; }
		auto camera() const -> camera_t const& { return camera_; }

		template <typename... Stages>
		auto draw(Stages&&... stages) -> void;

	private:
		static camera_t default_camera_;

		renderer_ptr rndr_;
		draw_target_t draw_target_;
		rendertarget_clear_t clear_;
		camera_t camera_;

		constant_buffer_ptr scene_constant_buffer_;

		draw_commands_t draw_commands_;

		friend struct renderer_t;
	};


	template <typename... Stages>
	auto scene_t::draw(Stages&&... stages) -> void
	{
		shiny::signal_draw(rndr_, draw_commands_, std::forward<Stages>(stages)...);
	}

}
