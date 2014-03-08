#include <dust/camera.hpp>
#include <DirectXMath.h>

namespace math = atma::math;
using namespace dust;
using dust::camera_t;


camera_t::camera_t()
	: view_(atma::math::look_at(math::point4f(), math::point4f(0.f, 0.f, 1.f), math::vector4f(0.f, 1.f, 0.f, 0.f))),
	  proj_(atma::math::perspective_fov(math::pi_over_two, 360/480, 0.03434f, 120.f))
{
	decompose_projection(fov_, aspect_, near_, far_);
}

camera_t::camera_t(atma::math::matrix4f const& view, atma::math::matrix4f const& proj)
	: view_(view), proj_(proj)
{
	decompose_projection(fov_, aspect_, near_, far_);
}

auto camera_t::view() const -> math::matrix4f const&
{
	return view_;
}

auto camera_t::projection() const -> math::matrix4f const&
{
	return proj_;
}

auto camera_t::fov() const -> float
{
	return 1.f;
}

auto camera_t::move_to(atma::math::vector4f const& p) -> void
{
	eye_ = p;

	view_ = atma::math::look_along(eye_, view_dir_, up_);
}

auto camera_t::look_at(atma::math::vector4f const& p) -> void
{
	view_dir_ = p - eye_;

	view_ = atma::math::look_along(eye_, view_dir_, up_);
}

auto camera_t::set_aspect(float r) -> void
{
	aspect_ = r;

	proj_ = atma::math::perspective_fov(fov_, aspect_, near_, far_);
}

auto camera_t::set_fov(float r) -> void
{
	fov_ = r;

	proj_ = atma::math::perspective_fov(fov_, aspect_, near_, far_);
}

auto camera_t::decompose_projection(float& fov, float& aspect, float& near, float& far) -> void
{
	fov = atan(1.f / proj_[0][0]) * 2.f;
	aspect = proj_[1][1] / proj_[0][0];
	near = 2.f * (-proj_[3][2] / (proj_[2][2] + 1.f));
	far = -proj_[3][2] / (proj_[2][2] - 1.f);
}
