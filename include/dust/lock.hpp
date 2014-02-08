#pragma once
//======================================================================
#include <atma/assert.hpp>

#include <d3d11.h>

#include <vector>
//======================================================================
namespace dust {
//======================================================================
	
	enum class lock_type_t
	{
		read,
		write,
		read_write,
		write_discard
	};

	template <typename resource_tm, typename element_tm>
	struct lock_t;

//======================================================================
} // namespace dust
//======================================================================
