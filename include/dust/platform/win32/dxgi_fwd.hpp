#pragma once
//======================================================================
#include <atma/types.hpp>
#include <atma/com_ptr.hpp>

#include <dust/surface_format.hpp>

#pragma warning(push,3)
#include <dxgi.h>
#pragma warning(pop)
#undef min
#undef max
//======================================================================
namespace dust { namespace platform {

	typedef atma::com_ptr<IDXGIFactory1> dxgi_factory_ptr;
	typedef atma::com_ptr<IDXGIAdapter1> dxgi_adapter_ptr;
	typedef atma::com_ptr<IDXGIOutput> dxgi_output_ptr;
	typedef atma::com_ptr<IDXGISwapChain> dxgi_swap_chain_ptr;


	inline auto dxgi_format_of(surface_format_t fmt) -> DXGI_FORMAT
	{
		static DXGI_FORMAT const mapping[] =
		{
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_TYPELESS,
			DXGI_FORMAT_R8G8B8A8_SINT,
			DXGI_FORMAT_R8G8B8A8_UINT,
			DXGI_FORMAT_R8G8B8A8_SNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_R32_TYPELESS,
		};

		return mapping[(uint)fmt];
	}

} }

