#pragma once

#include <shiny/buffer.hpp>

namespace shiny
{
	struct constant_buffer_t : buffer_t
	{
		constant_buffer_t(context_ptr const& context, size_t data_size, void const* data);
	};



	inline auto make_constant_buffer(context_ptr const& ctx, size_t data_size, void const* data) -> constant_buffer_ptr {
		return atma::make_intrusive<constant_buffer_t>(ctx, data_size, data);
	}

	template <typename T>
	inline auto make_constant_buffer(context_ptr const& context, T const& t) -> constant_buffer_ptr
	{
		return make_constant_buffer(context, sizeof(t), &t);
	}
}

