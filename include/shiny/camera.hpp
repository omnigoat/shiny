#pragma once

#include <atma/math/matrix4f.hpp>


namespace shiny
{
	struct camera_t
	{
		camera_t();
		camera_t(aml::matrix4f const& view, aml::matrix4f const& proj);

		auto view() const -> aml::matrix4f const&;
		auto inverse_view() const -> aml::matrix4f const&;
		auto projection() const -> aml::matrix4f const&;
		auto near_distance() const -> float;
		auto far_distance() const -> float;
		auto fov() const -> float;
		auto pitch() const -> float;
		auto yaw() const -> float;
		auto position() const -> aml::vector4f const& { return eye_; }

		auto move_to(aml::vector4f const&) -> void;
		auto look_at(aml::vector4f const&) -> void;
		auto look_along(aml::vector4f const&) -> void;
		auto move_and_look_at(aml::vector4f const& eye, aml::vector4f const& target) -> void;

		auto set_aspect(float ratio) -> void;
		auto set_fov(float radians) -> void;
		auto set_clipping(float near_plane, float far_plane) -> void;
		
		auto decompose_projection(float& fov, float& aspect, float& near_plane, float& far_plane) const -> void;

	private:
		mutable aml::matrix4f view_, proj_;
		mutable aml::matrix4f inverse_view_;

		aml::vector4f eye_, view_dir_, up_;
		float fov_, aspect_, near_, far_;

		mutable bool view_dirty_, proj_dirty_;
	};
}




