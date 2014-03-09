#include <dust/camera.hpp>
#include <DirectXMath.h>

namespace math = atma::math;
using namespace dust;
using dust::camera_t;


camera_t::camera_t()
	: camera_t(atma::math::look_at(math::point4f(), math::point4f(0.f, 0.f, 1.f), math::vector4f(0.f, 1.f, 0.f, 0.f)),
	   atma::math::perspective_fov(math::pi_over_two, 360/480, 0.03434f, 120.f))
{
}

camera_t::camera_t(atma::math::matrix4f const& view, atma::math::matrix4f const& proj)
	: view_(view), proj_(proj), view_dirty_(), proj_dirty_()
{
	// decompose view
	auto m = view.inverted();
	eye_ = atma::math::vector4f(m[3][0], m[3][1], m[3][2], 1.f);
	view_dir_ = atma::math::vector4f(m[2][0], m[2][1], m[2][2], 0.f);
	up_ = atma::math::vector4f(m[1][0], m[1][1], m[1][2], 0.f);

	decompose_projection(fov_, aspect_, near_, far_);
}

auto camera_t::view() const -> math::matrix4f const&
{
	if (view_dirty_) {
		view_ = atma::math::look_along(eye_, view_dir_, up_);
		proj_ = atma::math::perspective_fov(fov_, aspect_, near_, far_);
		view_dirty_ = false;
	}

	return view_;
}

auto camera_t::projection() const -> math::matrix4f const&
{
	if (proj_dirty_) {
		view_ = atma::math::look_along(eye_, view_dir_, up_);
		proj_ = atma::math::perspective_fov(fov_, aspect_, near_, far_);
		proj_dirty_ = false;
	}

	return proj_;
}

auto camera_t::fov() const -> float
{
	return fov_;
}

auto camera_t::move_to(atma::math::vector4f const& p) -> void
{
	eye_ = p;
	view_dirty_ = true;
}

auto camera_t::look_at(atma::math::vector4f const& p) -> void
{
	view_dir_ = p - eye_;
	view_dirty_ = true;
}

auto camera_t::move_and_look_at(atma::math::vector4f const& p, atma::math::vector4f const& t) -> void
{
	eye_ = p;
	view_dir_ = t - p;
	view_dirty_ = true;
}

auto camera_t::set_aspect(float r) -> void
{
	aspect_ = r;
	proj_dirty_ = true;
}

auto camera_t::set_fov(float r) -> void
{
	fov_ = r;
	proj_dirty_ = true;
}

auto camera_t::decompose_projection(float& fov, float& aspect, float& near, float& far) -> void
{
	fov = atan(1.f / proj_[0][0]) * 2.f;
	aspect = proj_[1][1] / proj_[0][0];
	near = 2.f * (-proj_[3][2] / (proj_[2][2] + 1.f));
	far = -proj_[3][2] / (proj_[2][2] - 1.f);
}
