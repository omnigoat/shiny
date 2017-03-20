#pragma once

#include <shiny/platform/dx11/d3d_fwd.hpp>
#include <shiny/shiny_fwd.hpp>
#include <shiny/resource_view.hpp>

#include <atma/intrusive_ptr.hpp>

#include <vector>


namespace shiny
{
	struct resource_dx11_t
	{
		virtual auto d3d_resource() const -> platform::d3d_resource_ptr = 0;
	};
}
