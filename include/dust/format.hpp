#ifndef DUST_FORMAT_HPP
#define DUST_FORMAT_HPP
//======================================================================
#include <cstdint>
//======================================================================
namespace dust {
//======================================================================
	
	enum class display_format_t
	{
		unknown,
		r32g32b32a32 = 1,
		r32g32b32a32_f32 = 2,
		r8g8b8a8_unorm = 28
	};

	struct display_mode_t
	{
		uint32_t width, height;
		uint32_t refreshrate_frames, refreshrate_period;
		display_format_t format;
	};

//======================================================================
} // namespace dust
//======================================================================
#endif
//======================================================================
