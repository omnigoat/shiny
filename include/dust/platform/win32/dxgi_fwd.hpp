#pragma once
//======================================================================
#include <atma/com_ptr.hpp>

#include <dxgi.h>
#undef min
#undef max
//======================================================================
namespace dust {
namespace platform {
//======================================================================
	
	typedef atma::com_ptr<IDXGIFactory1> dxgi_factory_ptr;
	typedef atma::com_ptr<IDXGIAdapter1> dxgi_adapter_ptr;
	typedef atma::com_ptr<IDXGIOutput> dxgi_output_ptr;
	typedef atma::com_ptr<IDXGISwapChain> dxgi_swap_chain_ptr;
	
//======================================================================
} // namespace platform
} // namespace dust
//======================================================================
