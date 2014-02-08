#include <dust/platform/win32/pixel_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::pixel_shader_t;


pixel_shader_t::pixel_shader_t(context_ptr const& context)
: context_(context)
{
	ID3DBlob* blob;
	ATMA_ENSURE_IS(S_OK, D3DCompileFromFile(L"../shaders/ps_basic.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &blob, nullptr));

	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, d3d_ps_.assign()));
}


