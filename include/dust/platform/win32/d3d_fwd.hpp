#pragma once
//======================================================================
#include <atma/com_ptr.hpp>

#include <d3d11.h>
#undef min
#undef max
//======================================================================
namespace dust {
namespace platform {
//======================================================================
	
	typedef atma::com_ptr<ID3D11Device> d3d_device_ptr;
	typedef atma::com_ptr<ID3D11DeviceContext> d3d_context_ptr;
	typedef atma::com_ptr<ID3D11Buffer> d3d_buffer_ptr;
	typedef atma::com_ptr<ID3D11InputLayout> d3d_input_layout_ptr;

//======================================================================
} // namespace platform
} // namespace dust
//======================================================================
