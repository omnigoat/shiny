#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WINDOWS
#	include <shiny_dx11/resource_view.hpp>
#endif

namespace shiny
{
	struct bound_input_views_t
	{
		bound_input_views_t() {}

		bound_input_views_t(std::initializer_list<bound_resource_view_t> list)
			: views(list.begin(), list.end())
		{
		}

		bound_resource_views_t views;
	};

	struct bound_compute_views_t
	{
		bound_compute_views_t() {}

		bound_compute_views_t(std::initializer_list<bound_resource_view_t> list)
			: views(list.begin(), list.end())
		{
		}

		bound_resource_views_t views;
	};
}