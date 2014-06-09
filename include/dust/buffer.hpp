#pragma once
//======================================================================
#include <atma/config/platform.hpp>

#ifdef ATMA_PLATFORM_WIN32
#	include <dust/platform/win32/buffer.hpp>
#endif

namespace dust
{
	struct generic_buffer_t : dust::buffer_t
	{
		
	private:
		
	};
}