#pragma once
//======================================================================
#include <atma/com_ptr.hpp>

#include <dust/dust_fwd.hpp>

#pragma warning(push,3)
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)
//======================================================================
#undef min
#undef max
//======================================================================
namespace dust { namespace platform {

	typedef atma::com_ptr<ID3D11Resource> d3d_resource_ptr;
	typedef atma::com_ptr<ID3D11Device> d3d_device_ptr;
	typedef atma::com_ptr<ID3D11DeviceContext> d3d_context_ptr;
	typedef atma::com_ptr<ID3D11Buffer> d3d_buffer_ptr;
	typedef atma::com_ptr<ID3D11InputLayout> d3d_input_layout_ptr;
	typedef atma::com_ptr<ID3D11GeometryShader> d3d_geometry_shader_ptr;
	typedef atma::com_ptr<ID3D11VertexShader> d3d_vertex_shader_ptr;
	typedef atma::com_ptr<ID3D11PixelShader> d3d_fragment_shader_ptr;
	typedef atma::com_ptr<ID3D11ComputeShader> d3d_compute_shader_ptr;
	typedef atma::com_ptr<ID3DBlob> d3d_blob_ptr;
	typedef atma::com_ptr<ID3D11RenderTargetView> d3d_render_target_view_ptr;
	typedef atma::com_ptr<ID3D11DepthStencilView> d3d_depth_stencil_buffer_ptr;
	typedef atma::com_ptr<ID3D11Texture2D> d3d_texture2d_ptr;
	typedef atma::com_ptr<ID3D11Texture3D> d3d_texture3d_ptr;
	typedef atma::com_ptr<ID3D11ShaderResourceView> d3d_shader_resource_view_ptr;
	typedef atma::com_ptr<ID3D11UnorderedAccessView> d3d_unordered_access_view_ptr;
	typedef atma::com_ptr<ID3D11View> d3d_view_ptr;

} }
