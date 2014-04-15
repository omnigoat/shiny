#pragma once
//======================================================================
#include <atma/config/platform.hpp>
//======================================================================
namespace dust
{
	enum class surface_format_t
	{
		unknown,
		r32g32b32a32,
		r32g32b32a32_f32,
		r8g8b8a8_unorm,

		generic_f32,
		generic_i8x4,
	};
}