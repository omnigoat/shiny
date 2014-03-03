#pragma once
//======================================================================
#include <atma/math/matrix4f.hpp>
//======================================================================
namespace dust {
//======================================================================

	struct camera_t
	{
		camera_t();
		camera_t(atma::math::matrix4f const& view, atma::math::matrix4f const& proj);

		auto view() const -> atma::math::matrix4f const&;
		auto projection() const -> atma::math::matrix4f const&;
		auto near_distance() const -> float;
		auto far_distance() const -> float;
		auto fov() const -> float;

		auto move_to(atma::math::vector4f const&) -> void;
		auto look_at(atma::math::vector4f const&) -> void;
		auto set_fov(float radians) -> void;
		auto set_clipping(float near, float far) -> void;

	private:
		atma::math::vector4f eye_, view_dir_, up_;
		atma::math::matrix4f view_, proj_;
	};

//======================================================================
}
//======================================================================
namespace shiny { using dust::camera_t; }



