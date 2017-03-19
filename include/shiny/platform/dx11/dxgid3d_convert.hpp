#pragma once

#include <shiny/platform/dx11/d3d_fwd.hpp>
#include <shiny/platform/dx11/dxgi_fwd.hpp>
#include <shiny/format.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/depth_state.hpp>


namespace shiny { namespace platform {
	
	namespace detail
	{
		template <typename T, int N, typename E>
		inline auto lookup(T(&map)[N], E x) -> T
		{
			ATMA_ASSERT(0 <= (int)x && (int)x < N);
			return map[(uint)x];
		}
	}

	inline auto dxgi_format_of(format_t fmt) -> DXGI_FORMAT
	{
		using ef = format_t;

		switch (fmt)
		{
			// unknown
			case ef::unknown: return DXGI_FORMAT_UNKNOWN;

			// 4-component
			case ef::g8x4: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
			case ef::u8x4: return DXGI_FORMAT_R8G8B8A8_UINT;
			case ef::s8x4: return DXGI_FORMAT_R8G8B8A8_SINT;
			case ef::nu8x4: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ef::ns8x4: return DXGI_FORMAT_R8G8B8A8_SNORM;
			case ef::f16x4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ef::f32x4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

			// 2-component
			case ef::u32x2: return DXGI_FORMAT_R32G32_UINT;
			case ef::f32x2: return DXGI_FORMAT_R32G32_FLOAT;

			// 1-component
			case ef::g32: return DXGI_FORMAT_R32_TYPELESS;
			case ef::u32: return DXGI_FORMAT_R32_UINT;
			case ef::s32: return DXGI_FORMAT_R32_SINT;
			case ef::u16: return DXGI_FORMAT_R16_UINT;

			// depth-stencil
			case ef::df32:    return DXGI_FORMAT_D32_FLOAT;
			case ef::dnu24s8: return DXGI_FORMAT_D24_UNORM_S8_UINT;

			default:
				ATMA_HALT("unsupported element-format");
				break;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	inline auto d3dbind_of(resource_type_t bu) -> D3D11_BIND_FLAG
	{
		static D3D11_BIND_FLAG const mapping[] =
		{
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_BIND_INDEX_BUFFER,
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_BIND_FLAG{},
			D3D11_BIND_FLAG{},
			D3D11_BIND_FLAG{},
			D3D11_BIND_FLAG{},
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

	inline auto d3d_comparison_of(comparison_t x) -> D3D11_COMPARISON_FUNC
	{
		static D3D11_COMPARISON_FUNC const mapping[] =
		{
			D3D11_COMPARISON_NEVER,
			D3D11_COMPARISON_LESS,
			D3D11_COMPARISON_LESS_EQUAL,
			D3D11_COMPARISON_EQUAL,
			D3D11_COMPARISON_NOT_EQUAL,
			D3D11_COMPARISON_GREATER_EQUAL,
			D3D11_COMPARISON_GREATER,
			D3D11_COMPARISON_ALWAYS,
		};

		return detail::lookup(mapping, x);
	}

	inline auto d3d_stencil_op_of(stencil_op_t x) -> D3D11_STENCIL_OP
	{
		static D3D11_STENCIL_OP const mapping[] =
		{
			D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_ZERO,
			D3D11_STENCIL_OP_INCR,
			D3D11_STENCIL_OP_DECR,
			D3D11_STENCIL_OP_INCR_SAT,
			D3D11_STENCIL_OP_DECR_SAT,
			D3D11_STENCIL_OP_REPLACE,
			D3D11_STENCIL_OP_INVERT,
		};

		return detail::lookup(mapping, x);
	}

}}
