#pragma once
//======================================================================
#include <dust/surface_format.hpp>

#include <atma/types.hpp>
#include <atma/config/platform.hpp>

//======================================================================
// display_mode_t is common to all outputs
//
// SERIOUSLY, find a better spot for this
//======================================================================
namespace dust
{
	struct display_mode_t
	{
		uint32 width, height;
		uint32 refreshrate_frames, refreshrate_period;
		surface_format_t format;
		bool fullscreen;
	};


	uint32 const primary_output = 0;
}

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/output.hpp>
#endif

