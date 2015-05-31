#pragma once

#include <atma/math/matrix4f.hpp>


namespace shiny
{
	struct camera_t
	{
		camera_t();
		camera_t(atma::math::matrix4f const& view, atma::math::matrix4f const& proj);

		auto view() const -> atma::math::matrix4f const&;
		auto projection() const -> atma::math::matrix4f const&;
		auto near_distance() const -> float;
		auto far_distance() const -> float;
		auto fov() const -> float;
		auto pitch() const -> float;
		auto yaw() const -> float;
		auto position() const -> aml::vector4f const& { return eye_; }

		auto move_to(atma::math::vector4f const&) -> void;
		auto look_at(atma::math::vector4f const&) -> void;
		auto look_along(atma::math::vector4f const&) -> void;
		auto move_and_look_at(atma::math::vector4f const& eye, atma::math::vector4f const& target) -> void;

		auto set_aspect(float ratio) -> void;
		auto set_fov(float radians) -> void;
		auto set_clipping(float near_plane, float far_plane) -> void;
		
		auto decompose_projection(float& fov, float& aspect, float& near_plane, float& far_plane) -> void;

	private:
		mutable atma::math::matrix4f view_, proj_;

		atma::math::vector4f eye_, view_dir_, up_;
		float fov_, aspect_, near_, far_;

		mutable bool view_dirty_, proj_dirty_;
	};
}




