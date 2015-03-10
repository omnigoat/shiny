#pragma once

#include <fooey/widgets/window.hpp>

#include <shiny/camera.hpp>


namespace pepper
{
	struct freelook_camera_controller_t
	{
		freelook_camera_controller_t(fooey::window_ptr const&);

		auto camera() const -> shiny::camera_t const&;

		auto update(uint timestep_in_ms) -> void;
		auto require_mousedown_for_rotation(bool) -> void;

	private:
		fooey::window_ptr window_;
		shiny::camera_t camera_;
		bool require_mousedown_;

		bool WASD_[4];
		std::atomic<bool> MB_;
		float phi_, theta_;
		int old_x_, old_y_;
		
		aml::vector4f position_;
		aml::vector4f walk_direction_;
		aml::vector4f strafe_direction_;
	};
}
