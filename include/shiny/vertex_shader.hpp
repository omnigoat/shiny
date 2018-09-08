#pragma once

#include <shiny/resource.hpp>


namespace shiny
{
	struct vertex_shader_t : component_t
	{
		vertex_shader_t(lion::path_t const& path, renderer_ptr const& rndr)
			: component_t{path, rndr}
		{}

		auto sizeof_host_resource() const -> size_t override { return sizeof(vertex_shader_t); }
	};

	template <typename T>
	using vertex_shader_bridge_t = device_bridge_t<vertex_shader_t, T>;
}
