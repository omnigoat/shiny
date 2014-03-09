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
		auto look_along(atma::math::vector4f const&) -> void;
		auto move_and_look_at(atma::math::vector4f const& eye, atma::math::vector4f const& target) -> void;

		auto set_aspect(float ratio) -> void;
		auto set_fov(float radians) -> void;
		auto set_clipping(float near, float far) -> void;
		
		auto decompose_projection(float& fov, float& aspect, float& near, float& far) -> void;

	private:
		mutable atma::math::matrix4f view_, proj_;

		// we have 64 bytes to play around with because of matrix alignment
		atma::math::vector4f eye_, view_dir_, up_;
		float fov_, aspect_, near_, far_;
		mutable bool view_dirty_, proj_dirty_;
	};

//======================================================================
}
//======================================================================
namespace shiny { using dust::camera_t; }



