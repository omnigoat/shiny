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
		constant_buffer_t(context_ptr const& context);

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

	private:
		context_ptr context_;

		platform::d3d_buffer_ptr d3d_buffer_;
	};

	auto create_constant_buffer(context_ptr const&) -> constant_buffer_ptr;

//======================================================================
} // namespace dust
//======================================================================
