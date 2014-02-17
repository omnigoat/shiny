#pragma once
//======================================================================
#include <dust/format.hpp>

#include <fooey/widgets/window.hpp>

#include <atma/com_ptr.hpp>


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
		display_format_t format;
		bool fullscreen;
	};


	uint32 const primary_output = 0;
}

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/output.hpp>
#endif

