#pragma once
//======================================================================
#include <atma/config/platform.hpp>
//======================================================================
namespace dust
{
	enum class surface_format_t
	{
		unknown,
		r32g32b32a32 = 1,
		r32g32b32a32_f32 = 2,
		r8g8b8a8_unorm = 28
	};
}

#if 0
#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/surface_format.hpp>
#endif
#endif
