#pragma once

#include <shiny_dx11/d3d_fwd.hpp>
#include <shiny/shiny_fwd.hpp>
#include <shiny/resource_view.hpp>

#include <atma/intrusive_ptr.hpp>

#include <vector>


namespace shiny_dx11
{
	struct resource_t
	{
		virtual auto d3d_resource() const -> d3d_resource_ptr = 0;
	};
}
