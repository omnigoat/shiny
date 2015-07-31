#pragma once

#include <atma/types.hpp>

namespace shiny
{
	//  how to read:
	//  --------------
	//    g: untyped (generic)
	//    s: signed integer
	//    u: unsigned integer
	//    f: floating-point
	//  
	//    n: normalised (0..255 interpreted as 0..1 in the shader)
	//  
	//    x4: 4 components
	//    x3: 3 components, etc
	//
	//
	//  bitfield:
	//  -----------
	//                      zzzzzzzz ccccnsif
	//    76543210 76543210 76543210 76543210
	//
	//    f: floating-point
	//    i: integer
	//    s: signed
	//    n: normalized
	//    c: component-count
	//    z: size of components
	//     : unused
	//
	#define  G 0x0 //0b0000
	#define  F 0x1 //0b0001
	#define  U 0x2 //0b0010
	#define  S 0x6 //0b0110
	#define NU 0xa //0b1010
	#define NS 0xe //0b1110

	#define MK(t, s, c) \
		((s << 8) | (c << 4) | (t))


	enum class element_format_t : uint32
	{
		unknown = 0,

		// 4-component
		g8x4  = MK( G, 8,  4),
		u8x4  = MK( U, 8,  4),
		s8x4  = MK( S, 8,  4),
		nu8x4 = MK(NU, 8,  4),
		ns8x4 = MK(NS, 8,  4),
		f16x4 = MK( F, 16, 4),
		f32x4 = MK( F, 32, 4),

		// 2-component
		u32x2 = MK(U, 32, 2),
		f32x2 = MK(F, 32, 2),

		// 1-component
		g32 = MK(G, 32, 1),
		u32 = MK(U, 32, 1),
		s32 = MK(S, 32, 1),
	};

	#undef MK
	#undef  G
	#undef  F
	#undef  U
	#undef  S
	#undef NU
	#undef NS


	auto element_count(element_format_t) -> uint;
	auto element_size(element_format_t) -> size_t;
	auto is_generic(element_format_t) -> bool;



	enum class index_format_t
	{
		index16,
		index32,
	};

	auto index_size(index_format_t) -> size_t;

}
