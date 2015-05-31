#include <pepper/freelook_camera_controller.hpp>

#include <fooey/events/mouse.hpp>
#include <fooey/keys.hpp>

using namespace pepper;
using pepper::freelook_camera_controller_t;

#pragma warning(disable: 4351)
freelook_camera_controller_t::freelook_camera_controller_t(fooey::window_ptr const& window)
	: window_(window)
	, require_mousedown_()
	, position_(0.f, 0.f, -2.f, 1.f)
	, walk_direction_(0.f, 0.f, 1.f, 0.f)
	, strafe_direction_(1.f, 0.f, 0.f, 0.f)
	, phi_(), theta_()
	, WASD_{}
	, MB_()
	, old_x_(-1), old_y_(-1)
{
	window_->on({
		{"mouse-enter", [&](fooey::events::mouse_t const& e) {
			old_x_ = e.x();
			old_y_ = e.y();
		}},

		{"mouse-move", [&](fooey::events::mouse_t const& e)
		{
			if (!require_mousedown_ || MB_)
			{
				auto dp = e.x() - old_x_;
				phi_ += dp * 0.01f;

				auto dt = e.y() - old_y_;
				theta_ -= dt * 0.01f;
			}

			old_x_ = e.x();
			old_y_ = e.y();
		}},

		{"mouse-down.left", [&]{
			MB_ = true;
		}},

		{"mouse-up.left", [&]{
			MB_ = false;
		}},

		{"mouse-leave", [&]{
			MB_ = false;
		}}
	});


	window->key_state.on_key_down(fooey::key_t::W, [&] { WASD_[0] = true; });
	window->key_state.on_key_down(fooey::key_t::A, [&] { WASD_[1] = true; });
	window->key_state.on_key_down(fooey::key_t::S, [&] { WASD_[2] = true; });
	window->key_state.on_key_down(fooey::key_t::D, [&] { WASD_[3] = true; });

	window->key_state.on_key_up(fooey::key_t::W, [&] { WASD_[0] = false; });
	window->key_state.on_key_up(fooey::key_t::A, [&] { WASD_[1] = false; });
	window->key_state.on_key_up(fooey::key_t::S, [&] { WASD_[2] = false; });
	window->key_state.on_key_up(fooey::key_t::D, [&] { WASD_[3] = false; });

#if 0
	camera_ = shiny::camera_t(
		aml::look_at(position_, position_ + walk_direction_, aml::vector4f {0.f, 1.f, 0.f, 0.f}),
		aml::perspective_fov(aml::pi_over_two, (float)window_->height() / window_->width(), 0.001f, 100.f));
#endif
}

auto freelook_camera_controller_t::camera() const -> shiny::camera_t const&
{
	return camera_;
}

auto freelook_camera_controller_t::update(uint timestep_in_ms) -> void
{
	if (MB_ || !require_mousedown_)
	{
		if (theta_ > aml::pi_over_two - 0.1)
			theta_ = aml::pi_over_two - 0.1f;
		else if (theta_ < -aml::pi_over_two + 0.1f)
			theta_ = -aml::pi_over_two + 0.1f;
	}

	walk_direction_ = aml::vector4f(sin(phi_) * cos(theta_), sin(theta_), cos(phi_) * cos(theta_), 0.f);
	strafe_direction_ = aml::cross_product(aml::vector4f(0.f, 1.f, 0.f, 0.f), walk_direction_);

	auto const walk_speed = 0.02f;
	if (WASD_[0]) position_ += walk_direction_ * walk_speed;
	if (WASD_[1]) position_ -= strafe_direction_ * walk_speed;
	if (WASD_[2]) position_ -= walk_direction_ * walk_speed;
	if (WASD_[3]) position_ += strafe_direction_ * walk_speed;

	camera_ = shiny::camera_t(
		aml::look_at(position_, position_ + walk_direction_, aml::vector4f{0.f, 1.f, 0.f, 0.f}),
		aml::perspective_fov(aml::pi_over_two, (float)window_->height() / window_->width(), 0.001f, 100.f));
}

auto freelook_camera_controller_t::require_mousedown_for_rotation(bool x) -> void
{
	require_mousedown_ = x;
}
