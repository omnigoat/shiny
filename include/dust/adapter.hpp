#pragma once
//======================================================================
#include <fooey/widgets/window.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/adapter.hpp>
#endif
//======================================================================
namespace dust {
//======================================================================

	enum class gpu_access_t
	{
		read,
		write,
		read_write
	};

	enum class cpu_access_t
	{
		none,
		read,
		write,
		read_write,
	};

//======================================================================
}
//======================================================================
