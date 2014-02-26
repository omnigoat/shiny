#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/dust_fwd.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/com_ptr.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust {
//======================================================================

	struct constant_buffer_t : atma::ref_counted
	{
		constant_buffer_t(context_ptr const& context, uint data_size, void* data);

		auto data_size() const -> uint;
		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

		auto signal_upload_new_data(void*) -> void;
		auto signal_upload_new_data(uint offset, uint size, void*) -> void;

	private:
		context_ptr context_;
		uint data_size_;

		platform::d3d_buffer_ptr d3d_buffer_;
	};

	auto create_constant_buffer(context_ptr const&, uint data_size, void* data) -> constant_buffer_ptr;

	template <typename T>
	inline auto create_constant_buffer(context_ptr const& context, T const& t) -> constant_buffer_ptr
	{
		return create_constant_buffer(context, sizeof t, &t);
	}

//======================================================================
} // namespace dust
//======================================================================
