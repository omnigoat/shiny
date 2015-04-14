#pragma once

#include <shiny/platform/win32/d3d_fwd.hpp>
#include <shiny/platform/win32/dxgi_fwd.hpp>
#include <shiny/element_format.hpp>
#include <shiny/blend_state.hpp>

namespace shiny { namespace platform {
	
	namespace detail
	{
		template <typename T, size_t N, typename E>
		inline auto lookup(T(&map)[N], E x) -> T
		{
			ATMA_ASSERT((size_t)x < N);
			return map[(uint)x];
		}
	}

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
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32_UINT,
		};

		return detail::lookup(mapping, fmt);
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

		return detail::lookup(mapping, bu);
	}

	inline auto d3dblend_of(blending_t b) -> D3D11_BLEND
	{
		static D3D11_BLEND const mapping[] =
		{
			D3D11_BLEND_ZERO,
			D3D11_BLEND_ONE,
			D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_DEST_COLOR,
			D3D11_BLEND_INV_DEST_COLOR,
			D3D11_BLEND_DEST_ALPHA,
			D3D11_BLEND_INV_DEST_ALPHA,
		};

		return detail::lookup(mapping, b);
	}

	inline auto d3d_input_class_of(data_stream_stage_t x) -> D3D11_INPUT_CLASSIFICATION
	{
		static D3D11_INPUT_CLASSIFICATION const mapping[] =
		{
			D3D11_INPUT_PER_INSTANCE_DATA,
			D3D11_INPUT_PER_VERTEX_DATA,
		};

		return detail::lookup(mapping, x);
	}

	inline auto dxgi_format_of(index_format_t x) -> DXGI_FORMAT
	{
		static DXGI_FORMAT const mapping[] =
		{
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_R32_UINT,
		};

		return detail::lookup(mapping, x);
	}

}}
