#pragma once
//======================================================================
#include <atma/types.hpp>
#include <atma/com_ptr.hpp>

#pragma warning(push,3)
#include <dxgi.h>
#pragma warning(pop)
#undef min
#undef max
//======================================================================
namespace shiny { namespace platform {

	typedef atma::com_ptr<IDXGIFactory1> dxgi_factory_ptr;
	typedef atma::com_ptr<IDXGIAdapter1> dxgi_adapter_ptr;
	typedef atma::com_ptr<IDXGIOutput> dxgi_output_ptr;
	typedef atma::com_ptr<IDXGISwapChain> dxgi_swap_chain_ptr;

} }

