#include <dust/platform/win32/vertex_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::vertex_shader_t;


vertex_shader_t::vertex_shader_t(context_ptr const& context)
: context_(context)
{
	ID3DBlob* blob;
	ATMA_ENSURE_IS(S_OK, D3DCompileFromFile(L"../shaders/vs_basic.hlsl", nullptr, nullptr, "main", "vs_4_0", 0, 0, &blob, nullptr));

	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, d3d_vs_.assign()));
}


