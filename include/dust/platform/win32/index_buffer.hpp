#pragma once
//======================================================================
#include <dust/buffer.hpp>
//======================================================================
namespace dust
{
	struct index_buffer_t : buffer_t
	{
		index_buffer_t(context_ptr const&, buffer_usage_t, uint index_size, uint index_count, void* data);
		~index_buffer_t();

		auto index_count() const -> uint { return index_count_; }

	private:
		uint index_count_;

		friend struct locked_index_buffer_t;
	};


	template <typename... Args>
	auto create_index_buffer(Args&&... args) -> index_buffer_ptr {
		return index_buffer_ptr(new index_buffer_t(std::forward<Args>(args)...));
	}
}

