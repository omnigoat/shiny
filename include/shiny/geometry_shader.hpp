#pragma once

#include <shiny/resource.hpp>


namespace shiny
{
	struct geometry_shader_t : component_t
	{
		geometry_shader_t(lion::path_t const& path, renderer_ptr const& rndr)
			: component_t{path, rndr}
		{}

		auto sizeof_host_resource() const -> size_t override { return sizeof(geometry_shader_t); }
	};

	template <typename T>
	using geometry_shader_bridge_t = device_bridge_t<geometry_shader_t, T>;
}
