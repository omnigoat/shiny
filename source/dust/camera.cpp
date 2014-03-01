#include <dust/camera.hpp>
#include <DirectXMath.h>

namespace math = atma::math;
using namespace dust;
using dust::camera_t;


camera_t::camera_t()
	: view_(atma::math::look_at(math::point4f(), math::point4f(0.f, 0.f, 1.f), math::vector4f(0.f, 1.f, 0.f, 0.f))),
	  proj_(atma::math::perspective_fov(math::pi_over_two, 1.f, 0.01f, 100.f))
{
}

camera_t::camera_t(atma::math::matrix4f const& view, atma::math::matrix4f const& proj)
	: view_(view), proj_(proj)
{
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
	return atan(1.f / proj_[1][1]) * 2.f;
}



