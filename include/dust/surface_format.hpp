#pragma once
//======================================================================
namespace dust
{
	// how to read:
	//  g: untyped (generic)
	//  s: signed integer
	//  u: unsigned integer
	//  f: floating-point
	//
	//  n: normalised (0..255 interpreted as 0..1 in the shader)
	//
	//  x4: 4 components
	//  x3: 3 components, etc
	//
	enum class surface_format_t
	{
		unknown,

		// 4-component
		g8x4, s8x4, u8x4, sn8x4, un8x4,
		f16x4, f32x4,

		// 1-component
		g32,
	};
}
