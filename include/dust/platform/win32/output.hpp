#pragma once
//======================================================================
#include <dust/platform/win32/dxgi_fwd.hpp>
#include <dust/dust_fwd.hpp>

#include <atma/com_ptr.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust
{
	struct output_t : atma::ref_counted
	{
		auto native_resolution() const -> std::pair<uint32, uint32>;
		
		auto dxgi_output() const -> platform::dxgi_output_ptr const& { return dxgi_output_; }

	private:
		platform::dxgi_output_ptr dxgi_output_;
	};
}
