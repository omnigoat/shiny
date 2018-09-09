#pragma once

#include <shiny/resource.hpp>


namespace shiny
{
	struct fragment_shader_t : component_t
	{
		fragment_shader_t(lion::path_t const& path, renderer_ptr const& rndr)
			: component_t{path, rndr}
		{}

		auto sizeof_host_resource() const -> size_t override { return sizeof(fragment_shader_t); }
	};

	template <typename T>
	using fragment_shader_bridge_t = device_bridge_t<fragment_shader_t, T>;
}
