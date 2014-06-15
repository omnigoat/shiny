#pragma once

#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/platform/win32/dxgi_fwd.hpp>
#include <dust/element_format.hpp>


namespace dust { namespace platform {
	
	inline auto dxgi_format_of(element_format_t fmt) -> DXGI_FORMAT
	{
		static DXGI_FORMAT const mapping[] =
		{
			DXGI_FORMAT_UNKNOWN,
			
			DXGI_FORMAT_R8G8B8A8_TYPELESS,
			DXGI_FORMAT_R8G8B8A8_SINT,
			DXGI_FORMAT_R8G8B8A8_UINT,
			DXGI_FORMAT_R8G8B8A8_SNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,

			DXGI_FORMAT_R32G32_UINT,

			DXGI_FORMAT_R32_TYPELESS,
		};

		ATMA_ASSERT((uint)fmt < std::extent<decltype(mapping)>::value);
		return mapping[(uint)fmt];
	}

	inline auto d3dbind_of(buffer_type_t bu) -> D3D11_BIND_FLAG
	{
		static D3D11_BIND_FLAG const mapping[] =
		{
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_BIND_INDEX_BUFFER,
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_BIND_SHADER_RESOURCE
		};

		ATMA_ASSERT((uint)bu < std::extent<decltype(mapping)>::value);
		return mapping[(uint)bu];
	}

}}
