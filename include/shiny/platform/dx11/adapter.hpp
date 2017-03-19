#pragma once
//======================================================================
#include <shiny/platform/dx11/dxgi_fwd.hpp>

#include <atma/com_ptr.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace shiny {
//======================================================================
	
	//======================================================================
	// adapter_t (d3d11/dxgi)
	// ------------------------
	//   represents one graphics card. wrapper around IDXGIAdapter.
	//======================================================================
	struct adapter_t;
	typedef atma::intrusive_ptr<adapter_t> adapter_ptr;

	struct adapter_t : atma::ref_counted
	{
		auto dxgi_adapter() const -> platform::dxgi_adapter_ptr const& { return dxgi_adapter_; }

	private:
		platform::dxgi_adapter_ptr dxgi_adapter_;
	};
	

	//======================================================================
	// primary-adapter
	//======================================================================
	//uint32 const primary_adapter = 0;

//======================================================================
} // namespace shiny
//======================================================================
