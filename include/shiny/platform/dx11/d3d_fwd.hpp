#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/com_ptr.hpp>

#pragma warning(push,3)
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)

#undef min
#undef max

namespace shiny::platform
{
	using d3d_device_ptr                = atma::com_ptr<ID3D11Device>;
	using d3d_renderer_ptr               = atma::com_ptr<ID3D11DeviceContext>;

	// shaders
	using d3d_blob_ptr                  = atma::com_ptr<ID3DBlob>;
	using d3d_vertex_shader_ptr         = atma::com_ptr<ID3D11VertexShader>;
	using d3d_geometry_shader_ptr       = atma::com_ptr<ID3D11GeometryShader>;
	using d3d_fragment_shader_ptr       = atma::com_ptr<ID3D11PixelShader>;
	using d3d_compute_shader_ptr        = atma::com_ptr<ID3D11ComputeShader>;

	// resources
	using d3d_resource_ptr              = atma::com_ptr<ID3D11Resource>;
	using d3d_buffer_ptr                = atma::com_ptr<ID3D11Buffer>;
	using d3d_texture2d_ptr             = atma::com_ptr<ID3D11Texture2D>;
	using d3d_texture3d_ptr             = atma::com_ptr<ID3D11Texture3D>;

	// views
	using d3d_view_ptr                  = atma::com_ptr<ID3D11View>;
	using d3d_render_target_view_ptr    = atma::com_ptr<ID3D11RenderTargetView>;
	using d3d_depth_stencil_view_ptr    = atma::com_ptr<ID3D11DepthStencilView>;
	using d3d_shader_resource_view_ptr  = atma::com_ptr<ID3D11ShaderResourceView>;
	using d3d_unordered_access_view_ptr = atma::com_ptr<ID3D11UnorderedAccessView>;

	// draw-state
	using d3d_input_layout_ptr          = atma::com_ptr<ID3D11InputLayout>;
	using d3d_blend_state_ptr           = atma::com_ptr<ID3D11BlendState>;
	using d3d_depth_stencil_state_ptr   = atma::com_ptr<ID3D11DepthStencilState>;

}

namespace shiny_d3d11
{
	// yes, in a header-file
	using namespace shiny;


	using d3d_device_ptr                = atma::com_ptr<ID3D11Device>;
	using d3d_renderer_ptr               = atma::com_ptr<ID3D11DeviceContext>;

	// shaders
	using d3d_blob_ptr                  = atma::com_ptr<ID3DBlob>;
	using d3d_vertex_shader_ptr         = atma::com_ptr<ID3D11VertexShader>;
	using d3d_geometry_shader_ptr       = atma::com_ptr<ID3D11GeometryShader>;
	using d3d_fragment_shader_ptr       = atma::com_ptr<ID3D11PixelShader>;
	using d3d_compute_shader_ptr        = atma::com_ptr<ID3D11ComputeShader>;

	// resources
	using d3d_resource_ptr              = atma::com_ptr<ID3D11Resource>;
	using d3d_buffer_ptr                = atma::com_ptr<ID3D11Buffer>;
	using d3d_texture2d_ptr             = atma::com_ptr<ID3D11Texture2D>;
	using d3d_texture3d_ptr             = atma::com_ptr<ID3D11Texture3D>;

	// views
	using d3d_view_ptr                  = atma::com_ptr<ID3D11View>;
	using d3d_render_target_view_ptr    = atma::com_ptr<ID3D11RenderTargetView>;
	using d3d_depth_stencil_view_ptr    = atma::com_ptr<ID3D11DepthStencilView>;
	using d3d_shader_resource_view_ptr  = atma::com_ptr<ID3D11ShaderResourceView>;
	using d3d_unordered_access_view_ptr = atma::com_ptr<ID3D11UnorderedAccessView>;

	// draw-state
	using d3d_input_layout_ptr          = atma::com_ptr<ID3D11InputLayout>;
	using d3d_blend_state_ptr           = atma::com_ptr<ID3D11BlendState>;
	using d3d_depth_stencil_state_ptr   = atma::com_ptr<ID3D11DepthStencilState>;

}
