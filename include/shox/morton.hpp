#pragma once

#include <atma/types.hpp>


namespace moxi
{
	namespace detail
	{
		extern uint32 const morton256_x[256];
		extern uint32 const morton256_y[256];
		extern uint32 const morton256_z[256];
	}

	auto morton_encoding(uint x, uint y, uint z) -> uint64;
	auto morton_decoding(uint64 morton, uint& x, uint& y, uint& z) -> void;
}
