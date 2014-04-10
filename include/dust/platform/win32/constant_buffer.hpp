#pragma once
//======================================================================
#include <dust/buffer.hpp>
//======================================================================
namespace dust
{
	struct constant_buffer_t : buffer_t
	{
		constant_buffer_t(context_ptr const& context, uint data_size, void const* data);
	};



	inline auto create_constant_buffer(context_ptr const& ctx, uint data_size, void const* data) -> constant_buffer_ptr {
		return constant_buffer_ptr(new constant_buffer_t(ctx, data_size, data));
	}

	template <typename T>
	inline auto create_constant_buffer(context_ptr const& context, T const& t) -> constant_buffer_ptr
	{
		return create_constant_buffer(context, sizeof t, &t);
	}
}

